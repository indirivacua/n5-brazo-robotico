from brdib import app
from flask import session, request
from flask import render_template, redirect, url_for, flash
from brdib.models import User, Drawing
from brdib.form import RegisterForm, LoginForm, UpdateForm
from brdib import db
from flask_login import login_user, logout_user, login_required
from brdib import bcrypt
import requests


# Página Principal (sin sesión activa)
@app.route('/')
def root_page():
    return render_template('root.html')


# Página Principal (con sesión activa)
@app.route('/home', methods=['GET', 'POST'])
@login_required
def home_page():
    if request.method == 'POST':
        if "form-submit-board" in request.form:
            draw_id = request.form["form-submit-board"]                                                                 # Obtiene el valor del atributo "value="
            draw = Drawing.query.filter_by(id=draw_id).first()
            #print("FORM BOARD home_page")
            #print(draw.x, draw.y, draw.type, draw.size)
            return redirect(url_for("board_page", posX=draw.x, posY=draw.y, type=draw.type, size=draw.size))            # redirect y no render porque sino se estaría simulando que estamos en board... (no andaría ningún botón)
    return render_template('home.html', drawings=Drawing.query.all())


# Página de Registro
@app.route('/register', methods=['GET', 'POST'])
def register_page():
    #pfp = ['static/pfp/default.jpg', 'static/pfp/arte.jpg', 'static/pfp/perro.jpg', 'static/pfp/montana.jpg', 'static/pfp/flores.jpg', 'static/pfp/conejo.jpg']
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
    #print(form.validate_on_submit())
    if form.validate_on_submit():
        #print(session['username'])
        user_to_update = User.query.filter_by(username=session['username']).first()
        #print(f'user_to_update.username: {user_to_update.username}')
        #print(f'form.username.data: {form.username.data}')
        if form.username.data != '':
            user_to_update.username = form.username.data
        #print(f'user_to_update.email: {user_to_update.email}')
        #print(f'form.email.data: {form.email.data}')
        if form.email.data != '':
            user_to_update.email = form.email.data
        #if form.pfp.data != '':
            #user_to_update.pfp = form.pfp.data
        user_to_update.password_hash = bcrypt.generate_password_hash(form.password_ok.data).decode('utf-8')
        if db.session.commit():
            session['username'] = form.username.data
            flash('Perfil modificado exitosamente.', category='info')
    if form.errors != {}:
        for error_message in form.errors.values():                                                                      # values() retorna una lista con los elementos del diccionario form.errors
            flash(error_message[0], category="danger")
    return render_template('settings.html', form=form)


# Página de Baja de Usuarios
@app.route('/unsubscribe')
@login_required
def unsubscribe_page():
    user_to_delete = User.query.filter_by(username=session['username']).first()
    drawings_to_delete = Drawing.query.filter_by(user_id=user_to_delete.username)
    logout_user()
    for drawing in drawings_to_delete:
        db.session.delete(drawing)
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


# Página de Pizarra
@app.route('/board', methods=["POST", "GET"])
@login_required
def board_page():
    if request.method == "POST":
        if "form-submit-save" in request.form:
            posX = request.form["save_posX"]
            posY = request.form["save_posY"]
            type = request.form["save_type"]
            size = request.form["save_size"]
            #print("FORM SAVE board_page")
            #print(int(posX), int(posY), type, int(size))
            draw = Drawing(session['username'], int(posX), int(posY), type, int(size))
            db.session.add(draw)
            db.session.commit()
            flash("Dibujo guardado correctamente!", category='success')
            return redirect(url_for('board_page')) # agregado para que desaparezcan los parámetros de la url
        elif "form-submit-print" in request.form:
            posX = request.form["print_posX"]
            posY = request.form["print_posY"]
            type = request.form["print_type"]
            size = request.form["print_size"]
            #print("FORM PRINT board_page")
            #print(posX, posY, type, size)
            draw = Drawing(session['username'], int(posX), int(posY), type, int(size))                                  # Cuando se imprime también se debe guardar
            db.session.add(draw)
            db.session.commit()
            flash("Dibujo guardado correctamente!", category='success')
            #r = requests.get('http://192.168.4.1/dimensions')
            #print(r.content) #x:30,y:70
            if posX != "":
                size = int(int(size)/10)
                posX = int(70 - int(posX)/10)
                posY = int(30 - int(posY)/10)
                try:
                    ploads = {'posX':posY, 'posY':posX, 'shape':type, 'size':size}
                    r = requests.get('http://192.168.4.1/draw?', params=ploads)
                    #print(r.content)
                    flash(r.content, category='info')
                except:
                    flash("No se encuentra conectado a la red 'Robotic Arm'!", category='danger')
            else:
                flash("No se puede imprimir un dibujo vacio!", category="danger")
            return redirect(url_for('board_page')) # agregado para que desaparezcan los parámetros de la url
    return render_template('board.html', posX=request.args.get('posX'), posY=request.args.get('posY'), type=request.args.get('type'), size=request.args.get('size')) # hubo que agregar todos los parámetros


# Página de Mis Dibujos
@app.route('/mydrawings', methods=["POST", "GET"])
@login_required
def mydrawings_page():
    if request.method == 'POST':
        if "form-submit-board" in request.form:
            draw_id = request.form["form-submit-board"] # obtiene el valor del atributo "value="
            draw = Drawing.query.filter_by(id=draw_id).first()
            #print("FORM BOARD mydrawings_page")
            #print(draw.x, draw.y, draw.type, draw.size)
            return redirect(url_for("board_page", posX=draw.x, posY=draw.y, type=draw.type, size=draw.size)) # redirect y no render porque sino se estaría simulando que estamos en board... (no andaría ningún botón)
        elif "form-submit-delete" in request.form:
            draw_id = request.form["form-submit-delete"]
            Drawing.query.filter_by(id=draw_id).delete()
            db.session.commit()
            flash("Se borró el dibujo correctamente!", category="success")
    return render_template('mydrawings.html', drawings=Drawing.query.filter_by(user_id=session['username']))
