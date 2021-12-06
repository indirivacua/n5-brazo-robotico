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
        if "dibujo-pizarra" in request.form:
            id_dibujo = request.form["dibujo-pizarra"]
            dibujo = Drawing.query.filter_by(id=id_dibujo).first()
            print("HOLA home_page")
            print(dibujo.x, dibujo.y, dibujo.type, dibujo.size)
            return render_template('board.html', coordX=dibujo.x, coordY=dibujo.y, figura=dibujo.type, tamanio=dibujo.size)
    return render_template('home.html', dibujos=Drawing.query.all())


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

# Página de Pizarra
@app.route('/board', methods=["POST", "GET"])
@login_required
def board_page():
    if "form-submit-guardar" in request.form:
        if request.method == "POST":
            coordX = request.form["guardar_coordX"]
            coordY = request.form["guardar_coordY"]
            figura = request.form["guardar_figura"]
            tamanio = request.form["guardar_tamanio"]
            print("FORM GUARDAR board_page")
            #print(coordX, coordY, figura, tamanio)
            print(int(coordX), int(coordY), figura, int(tamanio))
            dibujo = Drawing(session['username'], int(coordX), int(coordY), figura, int(tamanio))
            #print(type(coordX), type(coordY), type(figura), type(tamanio))
            print(type(int(coordX)), type(int(coordY)), type(figura), type(int(tamanio)))
            db.session.add(dibujo)
            db.session.commit()
            flash("Dibujo guardado correctamente!", category='success')
    else:
        if "form-submit-imprimir" in request.form:
            coordX = request.form["imprimir_coordX"]
            coordY = request.form["imprimir_coordY"]
            figura = request.form["imprimir_figura"]
            tamanio = request.form["imprimir_tamanio"]
            print("FORM IMPRIMIR board_page")
            print(coordX, coordY, figura, tamanio)
            flash("Dibujo enviado a imprimir correctamente!", category='success')
    return render_template('board.html')
