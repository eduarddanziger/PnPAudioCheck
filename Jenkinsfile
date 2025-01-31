def SOLUTION_NAME = 'PnPAudioCheck.sln'

pipeline {
    agent any

    environment {
       SIGN_KEY_CONTAINER = credentials('SignKeyContainer')
    }

    options {
        disableConcurrentBuilds()
        buildDiscarder(logRotator(numToKeepStr: '5'))
    }

    stages {
        stage('Build') {
            steps {
                // "c:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin\MSBuild.exe"
                bat "nuget restore \"${SOLUTION_NAME}\""
                bat "\"${tool 'MSBuild VS2022'}\" \"${SOLUTION_NAME}\" /p:Configuration=Release /target:Rebuild -restore"
                bat "dotnet publish \"Projects\\AudioClient\\AudioClient.csproj\" -c Release -p:PublishProfile=FolderProfile"
            }
        }
        stage('Post') {
            steps {
                chuckNorris()
            }
        }
        stage('Deploy') {
            steps {
                dir ("${WORKSPACE}\\artifacts") {
                    archiveArtifacts '*.*'
                }
                script {
                    def command=$/&git describe --tags --match Release-*.*.* --long/$
                    def LAST_RELEASE_TAG = powershell(returnStdout: true, script: command)
                    def OCCURANCE_OF_MINUS1 = LAST_RELEASE_TAG.indexOf("-")
                    OCCURANCE_OF_MINUS1 = LAST_RELEASE_TAG.indexOf("-", OCCURANCE_OF_MINUS1+1)
                    def OCCURANCE_OF_MINUS2 = LAST_RELEASE_TAG.indexOf("-", OCCURANCE_OF_MINUS1+1)
                    def RC_NUMBER_INTEGER = (LAST_RELEASE_TAG.substring(OCCURANCE_OF_MINUS1+1, OCCURANCE_OF_MINUS2)) as Integer
                    LAST_RELEASE_TAG = LAST_RELEASE_TAG.substring(0, OCCURANCE_OF_MINUS1)
                    if (RC_NUMBER_INTEGER > 0) {
                        def OCCURANCE_OF_LAST_POINT = LAST_RELEASE_TAG.indexOf(".")
                        OCCURANCE_OF_LAST_POINT = LAST_RELEASE_TAG.indexOf(".", OCCURANCE_OF_LAST_POINT+1)
                        def THIRD_NUMBER_INTEGER = (LAST_RELEASE_TAG.substring(OCCURANCE_OF_LAST_POINT+1)) as Integer
                        LAST_RELEASE_TAG = LAST_RELEASE_TAG.substring(0, OCCURANCE_OF_LAST_POINT+1) + (++THIRD_NUMBER_INTEGER).toString() + "-RC" + RC_NUMBER_INTEGER.toString().padLeft(3, "0")
                    }
                    currentBuild.description = "Release tag: ${LAST_RELEASE_TAG}"
                }
            }
        }
    }
}
