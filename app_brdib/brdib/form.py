from flask_wtf import FlaskForm                                                             # Importa la clase FlaskForm de flask_wtf, necesaria para los formularios de alta y moificación de usuarios.
from wtforms import StringField, PasswordField, RadioField, SubmitField
from wtforms.validators import Length, EqualTo, Email, DataRequired, ValidationError        # Importa validators para los campos de los formularios.
from brdib.models import User                                                               # Importa la clase User de brdib.models, necesaria para validación en el alta de usuarios.
import re


class RegisterForm(FlaskForm):
    email = StringField(label='Email', validators=[Email(message="El email ingresado no es válido."), DataRequired(message="Debe ingresar un email.")])
    username = StringField(label='Username', validators=[Length(min=2, max=15, message="El nombre de usuario debe tener entre 2 a 15 caracteres."), DataRequired(message="Debe ingresar un nombre de usuario.")])
    password_ok = PasswordField(label='Password_ok', validators=[Length(min=8, message="La contraseña debe tener al menos 8 caracteres."), DataRequired(message="Debe ingresar una contraseña.")])
    password_ch = PasswordField(label='Password_ch', validators=[EqualTo('password_ok', message="Las contraseñas ingresadas no coinciden."), DataRequired(message="Debe confirmar la contraseña.")])
    #pfp = StringField(label='Profile_picture')     # Link de pfp
    #pfp = RadioField('Label', choices=[('ph1'), ('ph2'), ('ph3'), ('ph4'), ('ph5'), ('ph6')])      # Pfp desde static
    submit = SubmitField(label='Registrarse')

    def validate_email(self, email_to_check):
        email = User.query.filter_by(email=email_to_check.data).first()
        if email:
            raise ValidationError('Dirección de electrónico en uso. Elija otra dirección.')

    def validate_username(self, username_to_check):
        user = User.query.filter_by(username=username_to_check.data).first()
        if user:
            raise ValidationError('Nombre de usuario en uso. Elija otro nombre de usuario.')

    '''
    VALIDACION DE LINK DE PFP

    def validate_pfp(self, pfp_to_check):
        if pfp_to_check.data != '':
            regex_url = 'https?://(?:[-\w.]|(?:%[\da-fA-F]{2}))+'
            regex_jpeg = '(?:([^:/?#]+):)?(?://([^/?#]*))?([^?#]*\.(?:jpg))(?:\?([^#]*))?(?:#(.*))?'
            if re.search(regex_url, pfp_to_check.data):
                if not re.search(regex_jpeg, pfp_to_check.data):
                    raise ValidationError('El formato de la imagen no es .jpeg.')
            else:
                raise ValidationError('El link ingresado no es válido.')
    '''


class LoginForm(FlaskForm):
    username = StringField(label='Username', validators=[DataRequired(message="Debe ingresar su nombre de usuario.")])
    password = PasswordField(label='Password_ok', validators=[DataRequired(message="Debe ingresar su contraseña.")])
    submit = SubmitField(label='Iniciar Sesión')


class UpdateForm(FlaskForm):
    email = StringField(label='Email')
    username = StringField(label='Username')
    password_ok = PasswordField(label='Password_ok', validators=[Length(min=8, message="La contraseña debe tener al menos 8 caracteres.")])
    password_ch = PasswordField(label='Password_ch', validators=[EqualTo('password_ok', message="Las contraseñas ingresadas no coinciden.")])
    #pfp = StringField(label='Profile_picture')
    submit = SubmitField(label='Actualizar')

    def validate_email(self, email_to_check):
        if email_to_check.data != '':
            regex = '^[a-z0-9]+[\._]?[a-z0-9]+[@]\w+[.]\w{2,3}$'
            if re.search(regex, email_to_check.data):
                email = User.query.filter_by(email=email_to_check.data).first()
                current_user_email = User.query.filter_by(email=email_to_check.data).first()
                if email and email != current_user_email:
                    raise ValidationError('Dirección de electrónico en uso. Elija otra dirección.')
            else:
                raise ValidationError('El email ingresado no es válido.')

    def validate_username(self, username_to_check):
        if username_to_check.data != '':
            username_to_check_len = len(username_to_check.data)
            if 2 <= username_to_check_len <= 15:
                user = User.query.filter_by(username=username_to_check.data).first()
                if user:
                    raise ValidationError('Nombre de usuario en uso. Elija otro nombre de usuario.')
            else:
                raise ValidationError('El nombre de usuario debe tener entre 2 a 15 caracteres.')

    '''
    VALIDACION DE LINK DE PFP

    def validate_pfp(self, pfp_to_check):
        if pfp_to_check.data != '':
            regex_url = 'https?://(?:[-\w.]|(?:%[\da-fA-F]{2}))+'
            regex_jpeg = '(?:([^:/?#]+):)?(?://([^/?#]*))?([^?#]*\.(?:jpg))(?:\?([^#]*))?(?:#(.*))?'
            if re.search(regex_url, pfp_to_check.data):
                if not re.search(regex_jpeg, pfp_to_check.data):
                    raise ValidationError('El formato de la imagen no es .jpeg.')
            else:
                raise ValidationError('El link ingresado no es válido.')
    '''
