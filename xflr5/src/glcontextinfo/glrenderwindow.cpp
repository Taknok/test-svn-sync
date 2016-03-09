/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
**
****************************************************************************/

#include "glrenderwindow.h"
#include <QTimer>
#include <QMatrix4x4>
#include <QOpenGLContext>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>


GLRenderWindow::GLRenderWindow(const QSurfaceFormat &format)
	: m_pContext(0),
      m_initialized(false),
      m_forceGLSL110(false),
      m_angle(0.0f)
{
    setSurfaceType(QWindow::OpenGLSurface);
    setFormat(format);
	m_pContext = new QOpenGLContext(this);
	m_pContext->setFormat(requestedFormat());
	if (!m_pContext->create())
	{
		delete m_pContext;
		m_pContext = 0;
    }
	xRot = yRot = zRot = 0;
	m_pOpenGLTexture = NULL;

}

GLRenderWindow::~GLRenderWindow()
{
	if(m_pOpenGLTexture) delete m_pOpenGLTexture;
}

void GLRenderWindow::exposeEvent(QExposeEvent *)
{
    if (isExposed())
        render();
}



// ES needs the precision qualifiers.
// On desktop GL QOpenGLShaderProgram inserts dummy defines for highp/mediump/lowp.
static const char *vertexShaderSource110 =
/*    "attribute highp vec4 posAttr;\n"
    "attribute lowp vec4 colAttr;\n"
    "varying lowp vec4 col;\n"
    "uniform highp mat4 matrix;\n"
    "void main() {\n"
    "   col = colAttr;\n"
    "   gl_Position = matrix * posAttr;\n"
	"}\n";*/
	"attribute highp vec4 vertex;\n"
	"attribute mediump vec4 texCoord;\n"
	"varying mediump vec4 texc;\n"
	"uniform highp mat4 matrix;\n"
	"void main(void)\n"
	"{\n"
	"    gl_Position = matrix * vertex;\n"
	"    texc = texCoord;\n"
	"}\n";
static const char *fragmentShaderSource110 =
/*    "varying lowp vec4 col;\n"
    "void main() {\n"
    "   gl_FragColor = col;\n"
	"}\n";*/
	"uniform sampler2D texture;\n"
	"varying mediump vec4 texc;\n"
	"void main(void)\n"
	"{\n"
	"    gl_FragColor = texture2D(texture, texc.st);\n"
	"}\n";

static const char *vertexShaderSource =
    "#version 150\n"
/*    "in vec4 posAttr;\n"
    "in vec4 colAttr;\n"
    "out vec4 col;\n"
    "uniform mat4 matrix;\n"
    "void main() {\n"
    "   col = colAttr;\n"
    "   gl_Position = matrix * posAttr;\n"
	"}\n";*/

	"in vec4 vertex;\n"
	"in vec4 texCoord;\n"
	"out vec4 texc;\n"
	"uniform mat4 matrix;\n"
	"void main(void)\n"
	"{\n"
	"    gl_Position = matrix * vertex;\n"
	"    texc = texCoord;\n"
	"}\n";

static const char *fragmentShaderSource =
    "#version 150\n"
/*    "in vec4 col;\n"
    "out vec4 fragColor;\n"
    "void main() {\n"
    "   fragColor = col;\n"
	"}\n";*/
	"uniform sampler2D texture;\n"
	"in vec4 texc;\n"
	"out vec4 fragColor;\n"
	"void main(void)\n"
	"{\n"
	"    fragColor = texture2D(texture, texc.st);\n"
	"}\n";



void GLRenderWindow::rotateBy(int xAngle, int yAngle, int zAngle)
{
	xRot += xAngle;
	yRot += yAngle;
	zRot += zAngle;
//	update();
}


void GLRenderWindow::init()
{
	m_pShaderProgram = new QOpenGLShaderProgram(this);

	QSurfaceFormat format = m_pContext->format();
    bool useNewStyleShader = format.profile() == QSurfaceFormat::CoreProfile;
    // Try to handle 3.0 & 3.1 that do not have the core/compatibility profile concept 3.2+ has.
    // This may still fail since version 150 (3.2) is specified in the sources but it's worth a try.
    if (format.renderableType() == QSurfaceFormat::OpenGL && format.majorVersion() == 3 && format.minorVersion() <= 1)
        useNewStyleShader = !format.testOption(QSurfaceFormat::DeprecatedFunctions);
    if (m_forceGLSL110)
        useNewStyleShader = false;

    const char *vsrc = useNewStyleShader ? vertexShaderSource : vertexShaderSource110;
    const char *fsrc = useNewStyleShader ? fragmentShaderSource : fragmentShaderSource110;
//	if(m_pOutStream) *m_pOutStream << "Using version %s shader", useNewStyleShader ? "150" : "110";
//	else
	QString strong = "Using version %s shader" +  useNewStyleShader ? "150" : "110";
	qDebug("Using version %s shader", useNewStyleShader ? "150" : "110");

	if (!m_pShaderProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vsrc)) {
		emit error(m_pShaderProgram->log());
        return;
    }
	if (!m_pShaderProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fsrc)) {
		emit error(m_pShaderProgram->log());
        return;
    }
	if (!m_pShaderProgram->link()) {
		emit error(m_pShaderProgram->log());
        return;
    }

	m_pShaderProgram->attributeLocation("vertex");
	m_pShaderProgram->attributeLocation("texCoord");
//	m_posAttr = m_pShaderProgram->attributeLocation("posAttr");
//	m_colAttr = m_pShaderProgram->attributeLocation("colAttr");
	m_matrixUniform = m_pShaderProgram->uniformLocation("matrix");

/*	m_vbo.create();
	m_vbo.bind();
	m_vbo.allocate(vertices, sizeof(vertices) + sizeof(colors));
	m_vbo.write(sizeof(vertices), colors, sizeof(colors));
	m_vbo.release();*/
	makeObject();

    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    if (m_vao.isCreated()) // have VAO support, use it
        setupVertexAttribs();
}


void GLRenderWindow::setupVertexAttribs()
{
    m_vbo.bind();
//	m_pShaderProgram->setAttributeBuffer(m_posAttr, GL_FLOAT, 0, 2);
//	m_pShaderProgram->setAttributeBuffer(m_colAttr, GL_FLOAT, sizeof(vertices), 3);
//	m_pShaderProgram->enableAttributeArray(m_posAttr);
//	m_pShaderProgram->enableAttributeArray(m_colAttr);

	m_pShaderProgram->enableAttributeArray("vertex");
	m_pShaderProgram->enableAttributeArray("texCoord");
	m_pShaderProgram->setAttributeBuffer("vertex", GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
	m_pShaderProgram->setAttributeBuffer("texCoord", GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

    m_vbo.release();
}


void GLRenderWindow::render()
{
	if (!m_pContext->makeCurrent(this)) {
        emit error(tr("makeCurrent() failed"));
        return;
    }

	QOpenGLFunctions *f = m_pContext->functions();
    if (!m_initialized) {
        m_initialized = true;
        f->glEnable(GL_DEPTH_TEST);
        f->glClearColor(0, 0, 0, 1);
		init();
        emit ready();
    }

    if (!m_vbo.isCreated()) // init() failed, don't bother with trying to render
        return;

    const qreal retinaScale = devicePixelRatio();
    f->glViewport(0, 0, width() * retinaScale, height() * retinaScale);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_pShaderProgram->bind();
/*	QMatrix4x4 matrix;
	matrix.perspective(60.0f, 4.0f / 3.0f, 0.1f, 100.0f);
	matrix.translate(0.0f, 0.0f, -2.0f);
	matrix.rotate(m_angle, 0.0f, 1.0f, 0.0f);
*/
	rotateBy(13, 15, 17);

	QMatrix4x4 pvmMatrix;
	pvmMatrix.ortho(-0.5f, +0.5f, +0.5f, -0.5f, 4.0f, 15.0f);
	pvmMatrix.translate(0.0f, 0.0f, -10.0f);
	pvmMatrix.rotate(xRot / 16.0f, 1.0f, 0.0f, 0.0f);
	pvmMatrix.rotate(yRot / 16.0f, 0.0f, 1.0f, 0.0f);
	pvmMatrix.rotate(zRot / 16.0f, 0.0f, 0.0f, 1.0f);

	m_pShaderProgram->setUniformValue(m_matrixUniform, pvmMatrix);

    if (m_vao.isCreated())
        m_vao.bind();
    else // no VAO support, set the vertex attribute arrays now
        setupVertexAttribs();

//    f->glDrawArrays(GL_TRIANGLES, 0, 3);

	m_pOpenGLTexture->bind();
	f->glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    m_vao.release();
	m_pShaderProgram->release();

    // swapInterval is 1 by default which means that swapBuffers() will (hopefully) block
    // and wait for vsync.
	m_pContext->swapBuffers(this);

    m_angle += 1.0f;

    // Instead of 0 wait a few more milliseconds before rendering again. This is
    // only here to make the UI widgets more responsive on slower machines. We
    // can afford it since our rendering is so lightweight.
	const int interval = 30;
    QTimer::singleShot(interval, this, SLOT(render()));
}




void GLRenderWindow::makeObject()
{
	static const int coords[4][3] =	{ { +1, -1, -1 }, { -1, -1, -1 }, { -1, +1, -1 }, { +1, +1, -1 } };

	m_pOpenGLTexture = new QOpenGLTexture(QImage(QString(":/images/xflr5_full.png")));

	QVector<GLfloat> vertData;
	for (int j=0; j<4; ++j) {
		// vertex position
		vertData.append(0.1 * coords[j][0]);
		vertData.append(0.4 * coords[j][1]);
		vertData.append(0.1 * coords[j][2]);
		// texture coordinate
		vertData.append(j == 0 || j == 1);
		vertData.append(j == 0 || j == 3);
	}

	m_vbo.create();
	m_vbo.bind();
	m_vbo.allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));
}

