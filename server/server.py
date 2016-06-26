from threading import Thread
from subprocess import check_output
from flask import Flask, render_template, request, redirect, url_for
app = Flask(__name__)

output = ""

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/start_thread')
def start_thread():
    Thread(target=simulate).start()
    return redirect(url_for('simulation_page'))

@app.route('/simulation_page')
def simulation_page():
    #if request.method == "GET":
    #    return render_template('simulation_page.html', output=output)
    return render_template('simulation_page.html', output=output)

def simulate():
    global output
    output = check_output(["../100prisoners", "83000000", "p", "4"]).decode().\
             replace('\n', '<br>')


if __name__ == '__main__':
    app.run(debug=True)
