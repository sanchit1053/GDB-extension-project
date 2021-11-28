import subprocess
import logging

# logging subprocess by https://gist.github.com/jaketame/3ed43d1c52e9abccd742b1792c449079
print("The server is running at http://127.0.0.1:5000/")

LOG_FILE = "test.log"
logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.INFO,filename=LOG_FILE,format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')


def main():

    process = subprocess.run(['python', 'application.py'], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    while process.poll() is None:
        while True:
            output =process.stdout.readline().decode()
            if output:
                logger.log(logging.INFO,output)
            else:
                break



try:
    main()
except:
    logging.warning("stopping")