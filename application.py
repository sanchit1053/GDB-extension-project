# from flask import Flask, render_template, request, jsonify
from flask import Flask, render_template, request, jsonify, redirect

import json
from pygdbmi.gdbcontroller import GdbController

#
import os
from werkzeug.utils import secure_filename
#

app = Flask(__name__)

#
dir = os.getcwd()
app.config["FILE_UPLOADS"] = os.path.join(dir ,"codefiles")
app.config["ALLOWED_FILE_EXTENSIONS"] = "EXE"

def allowed_file(filename):
     if not "." in filename:
        return False
     ext = filename.rsplit(".", 1)[1]

     if ext.upper() in app.config["ALLOWED_FILE_EXTENSIONS"]:
          return True
     else:
         return False


# #




@app.route("/", methods = ['POST' , 'GET'])
def index():
    # reading the sampple.json file which has the data about the graph
    with open('sample.json', 'r') as file:
        data = json.load(file)

    # if first time return the original data
    if request.method == 'GET':
        return render_template("index.html", data = (data))

    # if command made on page then process the commandand send back the response
    if request.method == 'POST':
        ###################
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
            print(dir)
            file_path =os.path.join(dir,file.filename)
#                with open(file_path,'r') as f:
#                    details = f.read().replace('\n','<br>')
            return render_template("index.html",data = (data))
        else:
########################
            # read the command
            command = request.form['cmd']
            # process the command
            response = gdb.write(command)        
            # return the json of the data
            return jsonify({'status': 200 , 'data' : data, 'response':response})
            return render_template("index.html", data = (data), response = response)

def apprun():   
    app.run(debug = True)

if __name__ == "__main__":
    # initializing a gdb controller to run commands
    gdb = GdbController();
    # adding the pyt file to the gdb
    gdb.write("source pyt.py")
    app.run(debug=True)
    # closing the gdb controller
    gdb.write("q")
