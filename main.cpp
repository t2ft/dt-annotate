// ***************************************************************************
// Annotate a reverse-compiled device tree
// ---------------------------------------------------------------------------
// application entry point
// ---------------------------------------------------------------------------
// Copyright (C) 2021 by t2ft - Thomas Thanner
// Waldstraße 15, 86399 Bobingen, Germany
// ---------------------------------------------------------------------------
// 2021-6-8  tt  Initial version created
// ***************************************************************************
#include "annotate.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("dt-annotate");
    QCoreApplication::setApplicationVersion("1.0.2 ");

    // parse command line
    QCommandLineParser parser;
    parser.setApplicationDescription("Annotate a reverse-compiled devide tree");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("in", QCoreApplication::translate("main", "re-compiled device tree source"));
    parser.addPositionalArgument("out", QCoreApplication::translate("main", "destination for annotated device-tree output"));
    QCommandLineOption beQiet("q", QCoreApplication::translate("main", "do not output any info"));
    parser.addOption(beQiet);
    parser.process(a);
    QStringList args = parser.positionalArguments();
    if (args.size()==0) {
        parser.showHelp(-2);
    }
    if (args.size()==1) {
        args << args[0] + ".annotated";
    }

    // annotate and exit
    int ret = 0;
    Annotate annotator(parser.isSet(beQiet));
    if (!annotator.process(args[0], args[1])) {
        qCritical() << annotator.errString();
        ret = -1;
    }
    return ret;
}
