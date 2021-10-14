from flask_wtf import FlaskForm
from wtforms import StringField, PasswordField, SubmitField
from wtforms.validators import Length, EqualTo, Email, DataRequired, ValidationError
from brdib.models import User


class RegisterForm(FlaskForm):

    def validate_email(self, email_to_check):
        email = User.query.filter_by(email=email_to_check.data).first()
        if email:
            raise ValidationError('Dirección de electrónico en uso. Elija otra dirección.')

    def validate_username(self, username_to_check):
        user = User.query.filter_by(username=username_to_check.data).first()
        if user:
            raise ValidationError('Nombre de usuario en uso. Elija otro nombre de usuario.')

    email = StringField(label='Email', validators=[Email(), DataRequired()])
    username = StringField(label='Username', validators=[Length(min=2, max=15), DataRequired()])
    password_ok = PasswordField(label='Password_ok', validators=[Length(min=8), DataRequired()])
    password_ch = PasswordField(label='Password_ch', validators=[EqualTo('password_ok'), DataRequired()])
    submit = SubmitField(label='Registrarse')


class LoginForm(FlaskForm):
    username = StringField(label='Username', validators=[DataRequired()])
    password = PasswordField(label='Password_ok', validators=[DataRequired()])
    submit = SubmitField(label='Iniciar Sesión')
