from flask_wtf import FlaskForm                                                             # Importa la clase FlaskForm de flask_wtf, necesaria para los formularios de alta y moificación de usuarios.
from wtforms import StringField, PasswordField, SubmitField
from wtforms.validators import Length, EqualTo, Email, DataRequired, ValidationError        # Importa validators para los campos de los formularios.
from brdib.models import User                                                               # Importa la clase User de brdib.models, necesaria para validación en el alta de usuarios.


class RegisterForm(FlaskForm):

    def validate_email(self, email_to_check):
        email = User.query.filter_by(email=email_to_check.data).first()
        if email:
            raise ValidationError('Dirección de electrónico en uso. Elija otra dirección.')

    def validate_username(self, username_to_check):
        user = User.query.filter_by(username=username_to_check.data).first()
        if user:
            raise ValidationError('Nombre de usuario en uso. Elija otro nombre de usuario.')

    email = StringField(label='Email', validators=[Email(message="El email ingresado no es válido."), DataRequired(message="Debe ingresar un email.")])
    username = StringField(label='Username', validators=[Length(min=2, max=15, message="El nombre de usuario debe tener entre 2 a 15 caracteres."), DataRequired(message="Debe ingresar un nombre de usuario.")])
    password_ok = PasswordField(label='Password_ok', validators=[Length(min=8, message="La contraseña debe tener al menos 8 caracteres."), DataRequired(message="Debe ingresar una contraseña.")])
    password_ch = PasswordField(label='Password_ch', validators=[EqualTo('password_ok', message="Las contraseñas ingresadas no coinciden."), DataRequired(message="Debe confirmar la contraseña.")])
    submit = SubmitField(label='Registrarse')


class LoginForm(FlaskForm):
    username = StringField(label='Username', validators=[DataRequired(message="Debe ingresar su nombre de usuario.")])
    password = PasswordField(label='Password_ok', validators=[DataRequired(message="Debe ingresar su contraseña.")])
    submit = SubmitField(label='Iniciar Sesión')


class UpdateForm(FlaskForm):

    def validate_email(self, email_to_check):
        email = User.query.filter_by(email=email_to_check.data).first()
        if email:
            raise ValidationError('Dirección de electrónico en uso. Elija otra dirección.')

    def validate_username(self, username_to_check):
        user = User.query.filter_by(username=username_to_check.data).first()
        if user:
            raise ValidationError('Nombre de usuario en uso. Elija otro nombre de usuario.')

    email = StringField(label='Email', validators=[Email(message="El email ingresado no es válido.")])
    username = StringField(label='Username', validators=[Length(min=2, max=15, message="El nombre de usuario debe tener entre 2 a 15 caracteres.")])
    password_ok = PasswordField(label='Password_ok', validators=[Length(min=8, message="La contraseña debe tener al menos 8 caracteres."), DataRequired(message="Debe ingresar una contraseña.")])
    password_ch = PasswordField(label='Password_ch', validators=[EqualTo('password_ok', message="Las contraseñas ingresadas no coinciden."), DataRequired(message="Debe confirmar la contraseña.")])
    submit = SubmitField(label='Actualizar')

