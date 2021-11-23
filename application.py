
from flask import Flask, render_template, request, jsonify, redirect
import json
from pygdbmi.gdbcontroller import GdbController
import os
from werkzeug.utils import secure_filename
app = Flask(__name__)

votes = 0

@app.route("/", methods = ['POST' , 'GET'])
def index():
    with open('sample.json', 'r') as file:
        data = json.load(file)
    if request.method == 'GET':
        return render_template("index.html", data = (data))
    if request.method == 'POST':
        if request.files:

                file = request.files["inpFile"]

                if file.filename == "":
                    print("Image must have a filename")
                    return redirect(request.url)

                if not allowed_file(file.filename):
                    print("This file extension is not supported")
                    return redirect(request.url)

                else:
                    filename = secure_filename(file.filename)
                  

                file.save(os.path.join(app.config["FILE_UPLOADS"], file.filename))

                print('File saved')
                print(file.filename)

                file_path = "/mnt/c/Users/capts/Desktop/gdb_extension/gdb_project/codefiles/" + file.filename

                print(file_path)

                with open(file_path,'r') as f:
                    details = f.read().replace('\n','<br>')

                return render_template("index.html",details = details,data = (data))
        else:
            command = request.form['cmd']
            response = gdb.write(command)

            return jsonify({'status': 200 , 'data' : data, 'response':response})
        #return render_template("index.html", data = (data), response = response)

app.config["FILE_UPLOADS"] = "/mnt/c/Users/capts/Desktop/gdb_extension/gdb_project/codefiles"
app.config["ALLOWED_FILE_EXTENSIONS"] = "CPP"

def allowed_file(filename):
    if not "." in filename:
        return False
    ext = filename.rsplit(".", 1)[1]

    if ext.upper() in app.config["ALLOWED_FILE_EXTENSIONS"]:
         return True
    else:
        return False

#{@app.route("/upload-file", methods = ['POST' , 'GET'])
#def upload_file():
#        if request.method == 'POST':
#
#            if request.files:

#                file = request.files["inpFile"]

#                if file.filename == "":
#                    print("Image must have a filename")
#                    return redirect(request.url)

#                if not allowed_file(file.filename):
#                    print("This file extension is not supported")
#                    return redirect(request.url)

#                else:
#                    filename = secure_filename(file.filename)
                  

#                file.save(os.path.join(app.config["FILE_UPLOADS"], file.filename))

#                print('File saved')
#                print(file.filename)

#                file_path = "/mnt/c/Users/capts/Desktop/gdb_extension/gdb_project/codefiles/" + file.filename

#                print(file_path)

#                with open(file_path,'r') as f:
#                    details = f.read().replace('\n','<br>')
#                    print(details)

#                return render_template("file_content.html", details=details)

#        return render_template("upload_file.html")



def apprun():   
    app.run(debug = True)

if __name__ == "__main__":
    gdb = GdbController();
    gdb.write("source pyt.py")
    app.run(debug=True)
    gdb.write("q")