from flask import Flask, render_template
import json
app = Flask(__name__)

votes = 0

@app.route("/")
def index():
    with open('sample.json', 'r') as file:
        data = json.load(file)
    return render_template("index.html", data = (data))