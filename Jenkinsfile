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

    // Overwrite in stages that need clang
    environment {
        // Default compiler. Override in each stage as needed.
        CC='gcc-6'
        CXX='g++-6'
        COMPILER='gcc-6'

        // Default parallelism for make
        MAKEJ='4'

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
                stage('Lint') {
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
                            builder.make(workDir: "build", target: "clang-format-all")
                        }
                        /** Run clang-format.
                         *  If the resulting 'git diff' is non-empty, then it found something,
                         *  so error out and display the diff.
                         */
                        sh '''#!/bin/bash -ex
                            git --no-pager diff --name-only HEAD > /tmp/clang-format-diff
                            if [ -s "/tmp/clang-format-diff" ]; then
                                echo "clang-format warning found"
                                git --no-pager diff
                                exit 1
                            fi
                            '''

                        sh '''#!/bin/bash -ex
                            (cd build && make clang-tidy-all)
                            if [ -e "fixes.yaml" ]; then
                                echo "clang-tidy warning found"
                                exit 1
                            fi
                            '''
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
