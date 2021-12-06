# COMMAND PROMPT:
#   set FLASK_APP=run.py
#   set FLASK_DEBUG=1
#   flask run

from brdib import app

if __name__ == '__main__':          # Comprueba si el archivo run.py se está ejecutando como programa principal.
    app.run(debug=True)
