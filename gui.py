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

        fname = QtGui.QFileDialog.getOpenFileName(self, 'Open file', './square.png')

        self.file_label.setText(fname)

        pixmap = QtGui.QPixmap(fname)
        scaled_pixmap = pixmap.scaled(self.image_label.size(), QtCore.Qt.KeepAspectRatio)
        self.image_label.setPixmap(scaled_pixmap)
        self.image_label.setAlignment(QtCore.Qt.AlignCenter)

    def gradient_descent(self):

        self.image = Image.open(str(self.file_label.text()))
        self.image = self.image.convert('RGB')
        self.image_color = self.image.convert('RGB')
        self.image_gray = ImageOps.grayscale(self.image)

        (width, height) = self.image.size

        u = [[0 for j in range(height + 2)] for i in range(width + 2)]
        pic = [[[0 for k in range(3)] for j in range(height + 2)] for i in range(width + 2)]

        # If white set to -1
        # If black set to +1

        for i in range(1, width + 1):
            for j in range(1, height + 1):
                print(i)
                pic[i][j][0], pic[i][j][1], pic[i][j][2] = self.image.getpixel((i - 1, j - 1))

                if pic[i][j][0] > 127 and pic[i][j][1] > 127 and pic[i][j][2] > 127:
                    u[i][j] = 1.0
                else:
                    u[i][j] = -1.0


        # print('\n'.join([''.join([' {:.0f} '.format(item) for item in row])
        #       for row in pic]))
        # print "\n\n"

        # Mirror image the border
        for i in range(len(u)):
            pic[i][0] = pic[i][1]
            pic[i][-1] = pic[i][-2]
            u[i][0] = u[i][1]
            u[i][-1] = u[i][-2]

        for j in range(len(u[0])):
            pic[0][j] = pic[1][j]
            pic[-1][j] = pic[-2][j]
            u[0][j] = u[1][j]
            u[-1][j] = u[-2][j]

        # Four corners
        u[0][0] = u[1][1]
        u[0][-1] = u[1][-2]
        u[-1][0] = u[-2][1]
        u[-1][-1] = u[-2][-2]

        t = 0
        count = 0

        while True:
            t += descent_func(pic, u)

            for i in range(width):
                for j in range(height):
                    self.paint_border(i+1,j+1,u)

            # Change to image_gray if using paint_black
            self.image.save("./output.png")

            if count % 100 == 0:
                self.image.save("./output/"+`count`+".png")

            pixmap = QtGui.QPixmap("./output.png")
            scaled_pixmap = pixmap.scaled(self.image_label.size(), QtCore.Qt.KeepAspectRatio)
            self.image_label.setPixmap(scaled_pixmap)
            self.image_label.setAlignment(QtCore.Qt.AlignCenter)
            print(t)
            QtGui.QApplication.processEvents()
            count += 1

    def paint_black(self,i,j,val):
        # 0 is black
        # 255 is white
        if val > 0:
            self.image_gray.putpixel((i,j), 0)
        else:
            self.image_gray.putpixel((i,j), 255)

    def paint_border(self,i,j,u):

        # Paint blue border is there are any white neighbouring pixels

        white_neighbours = 0

        if u[i][j] > 0:
            if u[i-1][j] < 0:
                white_neighbours += 1
            if u[i+1][j] < 0:
                white_neighbours += 1
            if u[i][j-1] < 0:
                white_neighbours += 1
            if u[i][j+1] < 0:
                white_neighbours += 1
            if u[i-1][j-1] < 0:
                white_neighbours += 1
            if u[i-1][j+1] < 0:
                white_neighbours += 1
            if u[i+1][j-1] < 0:
                white_neighbours += 1
            if u[i+1][j+1] < 0:
                white_neighbours += 1

        if white_neighbours > 0:
            self.image.putpixel((i-1,j-1), (0,0,255))
        else:
            self.image.putpixel((i-1,j-1), self.image_color.getpixel((i-1, j-1)))

def main():

    app = QtGui.QApplication(sys.argv)
    ex = GUI()
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
