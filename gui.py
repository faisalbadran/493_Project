import sys
from descent import descent_func
from PyQt4 import QtGui, QtCore
from PIL import Image
from PIL import ImageOps

class GUI(QtGui.QWidget):

    def __init__(self):
        super(GUI, self).__init__()
        picture = None
        self.initUI()

    def initUI(self):

        self.file_label = QtGui.QLabel()

        file_button = QtGui.QPushButton('Choose file')
        self.connect(file_button, QtCore.SIGNAL('clicked()'), self.show_dialog)

        play_button = QtGui.QPushButton('Start')
        self.connect(play_button, QtCore.SIGNAL('clicked()'), self.gradient_descent)

        self.image_label = QtGui.QLabel()


        grid = QtGui.QGridLayout()
        grid.setSpacing(10)

        grid.addWidget(file_button, 1, 0)
        grid.addWidget(self.file_label, 1, 1, 1, 4)
        grid.addWidget(play_button, 1, 5)
        grid.addWidget(self.image_label, 3, 0, 8, 6)

        self.setLayout(grid)

        self.setGeometry(300, 300, 1280, 720)
        self.setWindowTitle('493 Project')
        self.show()

    def show_dialog(self):

        fname = QtGui.QFileDialog.getOpenFileName(self, 'Open file', '/home')

        self.file_label.setText(fname)

        pixmap = QtGui.QPixmap(fname)
        scaled_pixmap = pixmap.scaled(self.image_label.size(), QtCore.Qt.KeepAspectRatio)
        self.image_label.setPixmap(scaled_pixmap)
        self.image_label.setAlignment(QtCore.Qt.AlignCenter)

    def gradient_descent(self):

        image = Image.open(str(self.file_label.text()))
        image = ImageOps.grayscale(image)

        (width, height) = image.size

        u = [[0 for j in range(height)] for i in range(width)]

        # If white set to -1
        # If black set to +1

        for i in range(width):
            for j in range(height):
                if image.getpixel((i, j)) > 127:
                    u[i][j] = 1.0
                else:
                    u[i][j] = -1.0

        pic = u
        t = 0

        while t <= 0.1:
            t += descent_func(pic, u)

            for i in range(width):
                for j in range(height):
                    if u[i][j] > 0:
                        image.putpixel((i,j), 255)
                    else:
                        image.putpixel((i,j), 0)

            image.save("./output.png")
            pixmap = QtGui.QPixmap("./output.png")
            scaled_pixmap = pixmap.scaled(self.image_label.size(), QtCore.Qt.KeepAspectRatio)
            self.image_label.setPixmap(scaled_pixmap)
            self.image_label.setAlignment(QtCore.Qt.AlignCenter)

def main():

    app = QtGui.QApplication(sys.argv)
    ex = GUI()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
