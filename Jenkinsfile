pipeline {
    agent any
    stages {
        stage('Run Script') {
            steps {
                sh '''
                    bash -c "source OptickEnv && echo 'Opticks Environment loaded'"
                '''
            }
        }
    }
}

