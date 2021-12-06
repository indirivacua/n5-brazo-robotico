from flask import Flask                         # Importa la clase Flask, necesaria para correr la aplicación web.
from flask_sqlalchemy import SQLAlchemy         # Importa la clase SQLAlchemy, necesaria para mapear las tablas de la BDD en objetos en Python.
from flask_bcrypt import Bcrypt                 # Importa la clase Bcrypt, necesaria para la encriptación de contraseñas de usuarios.
from flask_login import LoginManager            # Importa la clase LoginManager, necesaria para la administración de sesiones de usuarios.


app = Flask(__name__)                           # Crea una instancia de la clase Flask y le pasa como argumento el nombre de la aplicación,
                                                # donde __name__ es una variable especial de Python que contiene el nombre del módulo actual.
app.config['SQLALCHEMY_DATABASE_URI'] = \
    'sqlite:///brdib.db'
app.config['SECRET_KEY'] = \
    '4aba5ad158b07534c64d82d8'
db = SQLAlchemy(app)
bcrypt = Bcrypt(app)
login_manager = LoginManager(app)
login_manager.login_view = 'login_page'
login_manager.login_message_category = 'info'
login_manager.login_message = \
    'Debes iniciar sesión para acceder a esta página.'



from brdib import routes                        # Importa el archivo routes.py del paquete brdib
