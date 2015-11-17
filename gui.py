from PyQt4 import QtGui
import sys

def main():

    app = QtGui.QApplication(sys.argv)

    w = QtGui.QWidget()
    w.resize(1280, 720)
    w.setWindowTitle('493 Project')
    w.show()

    sys.exit(app.exec_())


if __name__ == '__main__':
    main()