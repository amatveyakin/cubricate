#include <QList>
#include <QString>
#include <QRegExp>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QMap>


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Parameters

const QChar   SPECIAL_CHAR = '\\';
const QString RX_MACROS_NAME = "";
const QString INCORRECT_COMMAND_MSG = "Incorrect ``%1'' command!";

const int     MAX_RECURSIVE_INCLUDES = 256;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Auxiliary

struct FileStackNode {
  QString fileName;
  int iLine;

  FileStackNode (const QString& fileName_, int iLine_) : fileName (fileName_), iLine (iLine_) { }
};

class FileStack : public QList <FileStackNode> {
public:
  QString toString () const {
    QString result;
    for (FileStack::ConstIterator it = constBegin(); it != constEnd(); ++it)
      result += (it == constEnd() - 1 ? "  at " : "from ") + it->fileName + ":" + QString::number (it->iLine + 1) + "\n";
    return result;
  }
};

class MyException : public QString {
public:
  MyException (const QString& errMsg) :
      QString (errMsg) { }

  MyException (const FileStack& fileStack, const QString& errMsg) :
      QString (fileStack.toString() + errMsg) {}

  MyException (const QString& fileName, int iLine, const FileStack& fileStack, const QString& errMsg) {
    FileStack fileStackCopy (fileStack);
    fileStackCopy.append (FileStackNode (fileName, iLine));
    *this = fileStackCopy.toString() + errMsg;
  }
};

typedef QMap <QString, QString> MacrosTable;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel

void processFile (const QString& fileName, FileStack& fileStack, MacrosTable& macrosTable, QTextStream& out) {
  QFileInfo fileInfo (fileName);
  if (fileStack.size () > MAX_RECURSIVE_INCLUDES) {
    throw MyException (fileStack,
                       "Unable to include file ``" + fileName + "'':\n"
                       "The limit of " + QString::number (MAX_RECURSIVE_INCLUDES) + " recursive includes reached!\n");
  }

  QFile fileIn (fileName);
  if (!fileIn.open (QIODevice::ReadOnly | QIODevice::Text)) {
    throw MyException (fileStack, "Unable to open file ``" + fileName + "''!\n");
  }

  QTextStream in (&fileIn);
  int iLine = 0;
  QString line = in.readLine();
  while (!line.isNull()) {
    line = line.trimmed();
    if (line[0] == SPECIAL_CHAR) {
      line = line.mid (1).trimmed();
      if (line.startsWith ("include")) {
        QRegExp rx ("^include\\s*\"([^\"]*)\"\\s*$");
        if (rx.indexIn (line) < 0)
          throw MyException (fileName, iLine, fileStack, INCORRECT_COMMAND_MSG.arg ("include"));
        QString includedFileName = rx.cap (1);
        fileStack.append (FileStackNode (fileName, iLine));
        QFileInfo includedFileInfo (fileInfo.path() + "/" + includedFileName);
        processFile (includedFileInfo.filePath (), fileStack, macrosTable, out);
        fileStack.removeLast();
      }
      else if (line.startsWith ("define")) {

      }
      else if (line.startsWith ("undef")) {
        QRegExp rx ("^undef\\s*(" + RX_MACROS_NAME + ")\\s*$");
        if (rx.indexIn (line) < 0)
          throw MyException (fileName, iLine, fileStack, INCORRECT_COMMAND_MSG.arg ("undef"));
        macrosTable.remove (rx.cap (1));
      }
    }
    else
      out << line << "\n";

    line = in.readLine();
    iLine++;
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Interface

void printUsage (QTextStream& out, const QString& programName) {
  out << "Usage:  " << programName << " input_file" << endl;
}

int main (int argc, char *argv[]) {
  QTextStream qIn  (stdin);
  QTextStream qOut (stdout);
  QTextStream qErr (stderr);

  if (argc != 2) {
    printUsage (qErr, argv[0]);
    return 1;
  }

  try {
    FileStack fileStack;
    MacrosTable macrosTable;
    processFile (argv[1], fileStack, macrosTable, qOut);
  }
  catch (MyException msg) {
    qErr << msg;
    return 1;
  }
  catch (...) {
    qErr << "Unknown error\n";
    return 1;
  }

  return 0;
}
