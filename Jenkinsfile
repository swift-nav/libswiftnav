#!groovy

/**
 * This Jenkinsfile will only work in a Swift Navigation build/CI environment, as it uses
 * non-public docker images and pipeline libraries.
 */

// Use 'ci-jenkins@somebranch' to pull shared lib from a different branch than the default.
// Default is configured in Jenkins and should be from "stable" tag.
@Library("ci-jenkins") import com.swiftnav.ci.*

def context = new Context(context: this)
context.setRepo("libswiftnav")
def builder = context.getBuilder()

bazelDockerImage = '571934480752.dkr.ecr.us-west-2.amazonaws.com/swift-build-bazel:2024-05-29'
modernDockerImage = '571934480752.dkr.ecr.us-west-2.amazonaws.com/swift-build-modern:2024-05-29'

/**
 * - Mount the refrepo to keep git operations functional on a repo that uses ref-repo during clone
 **/
String dockerMountArgs = "-v /mnt/efs/refrepo:/mnt/efs/refrepo"

pipeline {
    // Override agent in each stage to make sure we don't share containers among stages.
    agent any
    options {
        // Make sure job aborts after 2 hours if hanging.
        timeout(time: 2, unit: 'HOURS')
        timestamps()
        // Keep builds for 7 days.
        buildDiscarder(logRotator(daysToKeepStr: '7'))
    }

    stages {
        stage('Build') {
            parallel {
                stage('Unit Test') {
                    agent {
                        docker {
                            image modernDockerImage
                            args dockerMountArgs
                        }
                    }
                    environment {
                      CC = "gcc-11"
                      CXX = "g++-11"
                    }
                    steps {
                        gitPrep()
                        script {
                            builder.cmake()
                            builder.make(workDir: "build", target: "build-all-tests")
                        }
                        sh("./build/tests/test-swiftnav-common --gtest_output=xml")
                        // Convert the test results into a Jenkins-parsable junit-XML format
                        //sh("./tools/check2junit.py test_detail.xml > build/tests/test_results_junit.xml")
                        stash(
                                name: 'libswiftnavUnit',
                                includes: 'test_detail.xml')
                    }
                }
                stage('Bazel Build') {
                    agent {
                        docker {
                            image bazelDockerImage
                        }
                    }
                    steps {
                        gitPrep()
                        script {
                            sh('''#!/bin/bash -ex
                                | bazel build --subcommands --config=system //...
                                | bazel coverage --collect_code_coverage --combined_report=lcov --config=system //...
                                | genhtml bazel-out/_coverage/_coverage_report.dat -o coverage
                                | tar -zcvf coverage.tar.gz coverage/
                                |'''.stripMargin())
                        }
                    }
                    post {
                        always {
                            archiveArtifacts(artifacts: 'coverage.tar.gz', allowEmptyArchive: true)
                        }
                    }
                }
                stage('ASAN') {
                    agent {
                        docker {
                            image bazelDockerImage
                        }
                    }
                    steps {
                        gitPrep()
                        script {
                            sh('''#!/bin/bash -ex
                                | bazel test --config asan //...
                                |'''.stripMargin())
                        }
                    }
                    post {
                        always {
                            archiveArtifacts(artifacts: 'coverage.tar.gz', allowEmptyArchive: true)
                        }
                    }
                }
                stage('Format & Lint') {
                    agent {
                        docker {
                            image modernDockerImage
                            args dockerMountArgs
                        }
                    }
                    steps {
                        gitPrep()
                        script {
                            builder.cmake()
                            builder.make(workDir: "build", target: "clang-format-all-check")
                            builder.make(workDir: "build", target: "clang-tidy-all-check")
                        }
                    }
                    post {
                        always {
                            archiveArtifacts(artifacts: 'fixes.yaml', allowEmptyArchive: true)
                        }
                    }
                }
                stage('Code Coverage') {
                    agent {
                        docker {
                            image modernDockerImage
                            args dockerMountArgs
                        }
                    }
                    environment {
                      CC = "gcc-11"
                      CXX = "g++-11"
                    }
                    steps {
                        gitPrep()
                        script {
                            builder.cmake(buildType: "Debug", codeCoverage: "ON")
                            builder.make(workDir: "build", target: "ccov-all")
                        }
                    }
                    post {
                        success {
                            script {
                                context.uploadCodeCov(repo: "libswiftnav", searchdir: "build")
                            }
                        }
                    }
                }
            }
        }
    }
    post {
        always {
            node('linux') {
                unstash(name: "libswiftnavUnit")
                junit('*.xml')
            }
            script {
                context.slackNotify()
                context.postCommon()
            }
        }
    }
}
