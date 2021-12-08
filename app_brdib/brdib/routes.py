from brdib import app
from flask import session
from flask_session import Session
from flask import render_template, redirect, url_for, flash
from brdib.models import User
from brdib.form import RegisterForm, LoginForm, UpdateForm
from brdib import db
from flask_login import login_user, logout_user, login_required
from sqlalchemy.orm.attributes import flag_modified


# Página Principal (sin sesión activa)
@app.route('/')
def root_page():
    return render_template('root.html')


# Página Principal (con sesión activa)
@app.route('/home', methods=["POST", "GET"])
@login_required
def home_page():
    if request.method == 'POST':
        if "form-submit-board" in request.form:
            draw_id = request.form["form-submit-board"] # obtiene el valor del atributo "value="
            draw = Drawing.query.filter_by(id=draw_id).first()

            print("FORM BOARD home_page")
            print(draw.x, draw.y, draw.type, draw.size)

            return redirect(url_for("board_page", defPosX=draw.x, defPosY=draw.y, type=draw.type, size=draw.size)) # redirect y no render porque sino se estaría simulando que estamos en board... (no andaría ningún botón)
    return render_template('home.html', drawings=Drawing.query.all())


# Página de Registro
@app.route('/register', methods=['GET', 'POST'])
def register_page():
    form = RegisterForm()
    if form.validate_on_submit():
        user_to_create = User(email=form.email.data,
                              username=form.username.data,
                              password=form.password_ok.data)
        db.session.add(user_to_create)
        db.session.commit()
        login_user(user_to_create)
        session['username'] = form.username.data
        flash(f'Éxito! Iniciaste sesión como: {user_to_create.username}', category='success')
        return redirect(url_for('home_page'))
    if form.errors != {}:
        for error_message in form.errors.values():                                                                      # values() retorna una lista con los elementos del diccionario form.errors
            flash(error_message[0], category="danger")
    return render_template('register.html', form=form)


# Página de Inicio de Sesión
@app.route('/login', methods=['GET', 'POST'])
def login_page():
    form = LoginForm()
    if form.validate_on_submit():
        attempted_user = User.query.filter_by(username=form.username.data).first()
        if attempted_user and attempted_user.check_password_correction(attempted_password=form.password.data):
            login_user(attempted_user)
            flash(f'Éxito! Iniciaste sesión como: {attempted_user.username}', category='success')
            session['username'] = form.username.data
            return redirect(url_for('home_page'))
        else:
            flash(f'Ups! Usuario o contraseña incorrectos.', category='danger')
    return render_template('login.html', form=form)


# Página de Configuración (Modificar Perfil)
@app.route('/settings', methods=['GET', 'POST'])
@login_required
def settings_page():
    form = UpdateForm()
    if form.validate_on_submit():
        user_to_update = User.query.filter_by(username=session['username']).first()
        if form.username.data != '':
            user_to_update.username = form.username.data
            #flag_modified(user_to_update, 'username')
        if form.email.data != '':
            user_to_update.email = form.email.data
            #flag_modified(user_to_update, 'email')
        #db.session.merge(user_to_update)
        #db.session.flush()
        db.session.commit()
        session['username'] = form.username.data
        flash(f'Usuario modificado con éxito!', category='success')
        return redirect(url_for('home_page'))
    if form.errors != {}:
        for error_message in form.errors.values():                                                                      # values() retorna una lista con los elementos del diccionario form.errors
            flash(error_message[0], category="danger")
    return render_template('settings.html', form=form)


# Página de Baja de Usuarios
@app.route('/unsubscribe')
@login_required
def unsubscribe_page():
    user_to_delete = User.query.filter_by(username=session['username']).first()
    logout_user()
    db.session.delete(user_to_delete)
    db.session.commit()
    session.pop('username', None)
    flash('Baja de usuario realizada con éxito.', category='info')
    return redirect(url_for('root_page'))


# Página de Cierre de Sesión
@app.route('/logout')
@login_required
def logout_page():
    logout_user()
    session.pop('username', None)
    flash('Cerraste sesión correctamente.', category='info')
    return redirect(url_for('root_page'))

from flask import request
from brdib.models import Drawing
import requests

# Página de Pizarra
@app.route('/board', methods=["POST", "GET"])
@login_required
def board_page():
    if request.method == "POST":
        if "form-submit-save" in request.form:
            defPosX = request.form["save_defPosX"]
            defPosY = request.form["save_defPosY"]
            type = request.form["save_type"]
            size = request.form["save_size"]

            print("FORM SAVE board_page")
            print(int(defPosX), int(defPosY), type, int(size))

            draw = Drawing(session['username'], int(defPosX), int(defPosY), type, int(size))


            db.session.add(draw)
            db.session.commit()

            flash("Dibujo guardado correctamente!", category='success')

            return redirect(url_for('board_page')) # agregado para que desaparezcan los parámetros de la url
        elif "form-submit-print" in request.form:
            defPosX = request.form["print_defPosX"]
            defPosY = request.form["print_defPosY"]
            type = request.form["print_type"]
            size = request.form["print_size"]

            print("FORM PRINT board_page")
            print(defPosX, defPosY, type, size)

            #r = requests.get('http://192.168.4.1/dimensions')
            #print(r.content) #x:30,y:70

            if defPosX != "":
                size = int(int(size)/10)

                defPosX = int(70 - int(defPosX)/10)
                defPosY = int(30 - int(defPosY)/10)

                try:
                    ploads = {'posX':defPosY,'posY':defPosX,'shape':type,'size':size}
                    r = requests.get('http://192.168.4.1/draw?', params=ploads)
                    print(r.content)
                    flash(r.content, category='info')
                except:
                    flash("No se encuentra conectado a la red 'Robotic Arm'!", category='danger')

            else:
                flash("No se puede imprimir un dibujo vacio!", category="danger")

            return redirect(url_for('board_page')) # agregado para que desaparezcan los parámetros de la url
    return render_template('board.html', defPosX=request.args.get('defPosX'), defPosY=request.args.get('defPosY'), type=request.args.get('type'), size=request.args.get('size')) # hubo que agregar todos los parámetros
