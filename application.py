from flask import Flask, render_template, request
import json
from pygdbmi.gdbcontroller import GdbController
app = Flask(__name__)

votes = 0

@app.route("/", methods = ['POST' , 'GET'])
def index():
    with open('sample.json', 'r') as file:
        data = json.load(file)
    if request.method == 'GET':
        return render_template("index.html", data = (data))
    if request.method == 'POST':
        command = request.form['cmd']
        response = gdb.write(command)
        return render_template("index.html", data = (data), response = response)

def apprun():   
    app.run(debug = True)

if __name__ == "__main__":
    gdb = GdbController();
    gdb.write("source pyt.py")
    app.run(debug=True)
