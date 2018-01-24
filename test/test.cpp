#include "gui/GaussianBeamWindow.h"

#include <QtTest/QtTest>

class TestGaussianBeam: public QObject
{
Q_OBJECT

private slots:
	void checkSave();

private:
	void populateBench(OpticsBench* bench);
};

void TestGaussianBeam::populateBench(OpticsBench* bench)
{
	Fit* fit = bench->addFit(1);
	fit->setName("TestFit");
	fit->setColor(Qt::red);
}

void TestGaussianBeam::checkSave()
{
	// Save a bench
	GaussianBeamWindow window1;
	OpticsBench* bench1 = window1.bench();
	populateBench(bench1);
	window1.saveFile("unittest.xml");
	QVector<Fit> fits;
	for (int i = 0; i < bench1->nFit(); i++)
		fits.append(*bench1->fit(i));

	// Load it
	GaussianBeamWindow window2;
	window2.openFile("unittest.xml");
	OpticsBench* bench2 = window2.bench();

	// Check that the elements are recovered
	QCOMPARE(fits.size(), bench2->nFit());
	for (int i = 0; i < bench2->nFit(); i++)
		QVERIFY(fits[i] == *bench2->fit(i));
}

QTEST_MAIN(TestGaussianBeam)

#include "test.moc"

