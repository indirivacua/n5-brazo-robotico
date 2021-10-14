from brdib import app
from flask import render_template, redirect, url_for, flash
from brdib.models import User
from brdib.form import RegisterForm, LoginForm
from brdib import db
from flask_login import login_user, logout_user, login_required


# Página Principal (sin sesión activa)
@app.route('/')
def root_page():
    return render_template('root.html')


# Página Principal (con sesión activa)
@app.route('/home')
@login_required
def home_page():
    return render_template('home.html')


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
        flash(f'Éxito! Iniciaste sesión como: {user_to_create.username}', category='success')
        return redirect(url_for('home_page'))
    if form.errors != {}:
        for error_message in form.errors.values():
            flash(f'Error al crear usuario: {error_message}')
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
            return redirect(url_for('home_page'))
        else:
            flash(f'Ups! Usuario o contraseña incorrectos.', category='danger')
    return render_template('login.html', form=form)


# Página de Cierre de Sesión
@app.route('/logout')
@login_required
def logout_page():
    logout_user()
    flash('Cerraste sesión correctamente.', category='info')
    return redirect(url_for('root_page'))


# Página de Pizarra
@app.route('/board')
@login_required
def board_page():
    return render_template('board.html')