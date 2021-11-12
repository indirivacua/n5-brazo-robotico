from brdib import db, login_manager
from brdib import bcrypt
from flask_login import UserMixin


@login_manager.user_loader
def load_user(user_id):
    return User.query.get(int(user_id))

# Modelo de tabla de Usuarios
class User(db.Model, UserMixin):
    id = db.Column(db.Integer(), primary_key=True)
    email = db.Column(db.String(length=50), nullable=False, unique=True)
    username = db.Column(db.String(length=30), nullable=False, unique=True)
    password_hash = db.Column(db.String(length=60), nullable=False)
    #pfp = db.Column(db.String(), nullable=False)
    drawings = db.relationship('Drawing', backref='user', lazy=True)

    @property
    def password(self):
        return self.password

    @password.setter
    def password(self, plain_text_password):
        self.password_hash = bcrypt.generate_password_hash(plain_text_password).decode('utf-8')

    def check_password_correction(self, attempted_password):
        return bcrypt.check_password_hash(self.password_hash, attempted_password)

# Modelo de tabla de Dibujos
class Drawing(db.Model):
    id = db.Column(db.Integer(), primary_key=True)
    user_id = db.Column(db.Integer, db.ForeignKey('user.id'), nullable=False)
    x = db.Column(db.Integer(), nullable=False)
    y = db.Column(db.Integer(), nullable=False)
    type = db.Column(db.Integer(), nullable=False)
    size = db.Column(db.Integer(), nullable=False)


