#include <OpenGL/gl.h>
#include "Window.h"

#include <vector>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <QDockWidget>
#include <QGroupBox>
#include <QButtonGroup>
#include <QMenuBar>
#include <QApplication>
#include <QLayout>
#include <QLabel>
#include <QProgressBar>
#include <QCheckBox>
#include <QRadioButton>
#include <QColorDialog>
#include <QLCDNumber>
#include <QPixmap>
#include <QFrame>
#include <QSplitter>
#include <QMenu>
#include <QScrollArea>
#include <QCoreApplication>
#include <QFont>
#include <QSizePolicy>
#include <QImageReader>
#include <QStatusBar>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QStatusBar>

using namespace std;


Window::Window () : QMainWindow (NULL) {
    try {
        viewer = new GLViewer;
    } catch (GLViewer::Exception e) {
        cerr << e.getMessage () << endl;
        exit (1);
    }
    setCentralWidget (viewer);
    QDockWidget * controlDockWidget = new QDockWidget (this);
    initControlWidget ();
    
    controlDockWidget->setWidget (controlWidget);
    controlDockWidget->adjustSize ();
    addDockWidget (Qt::RightDockWidgetArea, controlDockWidget);
    controlDockWidget->setFeatures (QDockWidget::AllDockWidgetFeatures);
    statusBar()->showMessage("");
    cout << viewer->size().width() << endl;
    
    //viewer->setBackgroundColor( QColor(255,243,232)); // couleur rosée
    viewer->setBackgroundColor( QColor(88, 175, 185)); // couleur bleu
    viewer->updateGL();
    
}

Window::~Window () {

}

void Window::setBGColor () {
    QColor c = QColorDialog::getColor (QColor (200, 200, 200), this);
    if (c.isValid () == true) {
        cout << c.red () << endl;
        viewer->setBackgroundColor (c);
        viewer->updateGL ();
    }
}

void Window::exportGLImage () {
    viewer->saveSnapshot (false, false);
}

void Window::initControlWidget () {
    controlWidget = new QGroupBox ();
    QVBoxLayout * layout = new QVBoxLayout (controlWidget);
    
    QGroupBox * previewGroupBox = new QGroupBox ("Preview", controlWidget);
    QVBoxLayout * previewLayout = new QVBoxLayout (previewGroupBox);
    
    QCheckBox * wireframeCheckBox = new QCheckBox ("Wireframe", previewGroupBox);
    connect (wireframeCheckBox, SIGNAL (toggled (bool)), viewer, SLOT (setWireframe (bool)));
    previewLayout->addWidget (wireframeCheckBox);
    
    QCheckBox * influenceArea = new QCheckBox ("Bone's influencial area", previewGroupBox);
    connect (influenceArea, SIGNAL(toggled(bool)), viewer, SLOT(setInfluenceArea(bool)));
    previewLayout->addWidget(influenceArea);
    
    QCheckBox * boneHide = new QCheckBox ("Hide bones", previewGroupBox);
    connect (boneHide, SIGNAL(toggled(bool)), viewer, SLOT(setBoneVisualisation(bool)));
    previewLayout->addWidget(boneHide);
   
    QButtonGroup * modeButtonGroup = new QButtonGroup (previewGroupBox);
    modeButtonGroup->setExclusive (true);
    QRadioButton * flatButton = new QRadioButton ("Flat", previewGroupBox);
    QRadioButton * smoothButton = new QRadioButton ("Smooth", previewGroupBox);
    modeButtonGroup->addButton (flatButton, static_cast<int>(GLViewer::Flat));
    modeButtonGroup->addButton (smoothButton, static_cast<int>(GLViewer::Smooth));
    connect (modeButtonGroup, SIGNAL (buttonClicked (int)), viewer, SLOT (setRenderingMode (int)));
    smoothButton->setChecked(true);
    previewLayout->addWidget (flatButton);
    previewLayout->addWidget (smoothButton);
    
    QPushButton * snapshotButton  = new QPushButton ("Load mesh and bones", previewGroupBox);
    connect (snapshotButton, SIGNAL (clicked ()) , viewer, SLOT (loadMesh ()));
    previewLayout->addWidget (snapshotButton);
    
    QPushButton * saveMeshButton = new QPushButton ("Save mesh and bones", previewGroupBox);
    connect (saveMeshButton, SIGNAL(clicked()), viewer, SLOT (exportMesh() ) );
    previewLayout->addWidget(saveMeshButton);
    
    QPushButton * init = new QPushButton ("Recharger le mesh", previewGroupBox);
    connect (init, SIGNAL(clicked()), viewer, SLOT (reinit()) );
    previewLayout->addWidget (init);

    layout->addWidget (previewGroupBox);
    
    QGroupBox * globalGroupBox = new QGroupBox ("Global Settings", controlWidget);
    QVBoxLayout * globalLayout = new QVBoxLayout (globalGroupBox);
    
    QButtonGroup * modeGroup = new QButtonGroup (globalGroupBox);
    modeGroup->setExclusive (true);
    QRadioButton * selectMode = new QRadioButton ("Selection mode", globalGroupBox);
    QRadioButton * standardMode = new QRadioButton ("Standard mode", globalGroupBox);
    
    QWidget * box = new QWidget(globalGroupBox);
    QHBoxLayout * suppr_layout = new QHBoxLayout(box);
    QPushButton * suppr = new QPushButton("Delete bone", box);
    connect(suppr, SIGNAL(clicked()), viewer, SLOT(supprBone()));
    QRadioButton * editMode = new QRadioButton ("Edit Mode", box);
    suppr_layout->addWidget(editMode);
    suppr_layout->addWidget(suppr);
    
    modeGroup->addButton(selectMode, static_cast<int>(GLViewer::Select));
    modeGroup->addButton(standardMode, static_cast<int>(GLViewer::Standard));
    modeGroup->addButton(editMode, static_cast<int>(GLViewer::Edit));
    connect(modeGroup, SIGNAL(buttonClicked(int)), viewer, SLOT(setSelectionMode(int)));
    standardMode->setChecked(true);
    globalLayout->addWidget(standardMode);
    globalLayout->addWidget(selectMode);
    globalLayout->addWidget(box);
    
    
    QPushButton * bgColorButton  = new QPushButton ("Background Color", globalGroupBox);
    connect (bgColorButton, SIGNAL (clicked()) , this, SLOT (setBGColor()));
    globalLayout->addWidget (bgColorButton);
    
    QPushButton * quitButton  = new QPushButton ("Quit", globalGroupBox);
    connect (quitButton, SIGNAL (clicked()) , qApp, SLOT (closeAllWindows()));
    globalLayout->addWidget (quitButton);

    layout->addWidget (globalGroupBox);

    layout->addStretch (0);
}
