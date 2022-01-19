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
                        dockerfile {
                            args dockerMountArgs
                        }
                    }
                    steps {
                        gitPrep()
                        script {
                            builder.cmake()
                            builder.make(workDir: "build")
                        }
                        // Convert the test results into a Jenkins-parsable junit-XML format
                        sh("./tools/check2junit.py build/tests/test_results.xml > build/tests/test_results_junit.xml")
                        stash(
                                name: 'libswiftnavUnit',
                                includes: 'build/tests/test_results_junit.xml')
                    }
                }
                stage('Bazel Build') {
                    agent {
                        docker {
                            image '571934480752.dkr.ecr.us-west-2.amazonaws.com/swift-build-bazel:2022-09-09'
                        }
                    }
                    steps {
                        gitPrep()
                        script {
                            try {
                                sh('''#!/bin/bash -ex
                                    | CC=gcc-8 CXX=g++-8 bazel build --subcommands //...
                                    | bazel run //:refresh_compile_commands
                                    | bazel run //:swiftnav-test
                                    |'''.stripMargin())
                            } catch(e) {
                                // Notify bazel-alerts when master fails
                                if (context.isBranchPush(branches: ["master"])) {
                                    slackSend(
                                        channel: "#bazel-alerts",
                                        color: 'danger',
                                        message: 'FAILURE'
                                            + " on master "
                                            + ": <${env.RUN_DISPLAY_URL}|${env.JOB_NAME} #${env.BUILD_NUMBER}>"
                                            + " - "
                                            + currentBuild.durationString.replace(' and counting', ''))
                                }
                            }
                        }
                    }
                }
                stage('Format & Lint') {
                    agent {
                        dockerfile {
                            filename "Dockerfile.modern"
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
                        dockerfile {
                            args dockerMountArgs
                        }
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
                junit('build/tests/*.xml')
            }
            script {
                context.slackNotify()
                context.postCommon()
            }
        }
    }
}
