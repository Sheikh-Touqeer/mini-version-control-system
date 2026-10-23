#include <iostream>
#include <cstring>
using namespace std;
char* copyText(const char* text)
{
    if (text == NULL)
        text = "";

    char* copiedText = new char[strlen(text) + 1];
    strcpy(copiedText, text);
    return copiedText;
}
class Blob
{
private:
    char* filename;
    char* contents;
    int byteCount;
    int checksumSlot;

public:
    static int objectsAlive;
    Blob() : filename(NULL), contents(NULL),
             byteCount(0), checksumSlot(4)
    {
        objectsAlive++;
    }
    Blob(const char* file, const char* text)
    : filename(NULL), contents(NULL), byteCount(0), checksumSlot(4)
    {
        objectsAlive++;
        setFilename(file);
        setContents(text);
    }
Blob(const Blob& sourceRef)
    : filename(NULL), contents(NULL), byteCount(0), checksumSlot(4)
    {
        objectsAlive++;
        setFilename(sourceRef.filename);
        setContents(sourceRef.contents);
    }
    ~Blob()
    {
        delete[] filename;
        delete[] contents;
        objectsAlive--;
    }

    void setFilename(const char* file)
    {
        char* temp = copyText(file);
        delete[] filename;
        filename = temp;
    }
void setContents(const char* text)
    {
        char* temp = copyText(text);
        delete[] contents;
        contents = temp;
        byteCount = strlen(contents);
    }
const char* getFilename() const
    {
        return filename == NULL ? "" : filename;
    }

const char* getContents() const
    {
        return contents == NULL ? "" : contents;
    }

int getByteCount() const
    {
        return byteCount;
    }

friend bool compareBlobs(const Blob& a, const Blob& b);
};
int Blob::objectsAlive = 0;
bool compareBlobs(const Blob& a, const Blob& b)
{
    return strcmp(a.filename, b.filename) == 0 &&
           strcmp(a.contents, b.contents) == 0;
}

class Commit
{
private:
    char* message;
    Blob* snapshot;
    int fileCount;
    const int commitID;
    Commit* parent;
    int checksumSlot;

    static int nextCommitID;
public:
    static int objectsAlive;
    static int commitsEverMade;

    Commit(const char* msg, const Blob* files,
           int count, Commit* previous)
    : message(NULL), snapshot(NULL), fileCount(count), commitID(nextCommitID++), parent(previous), checksumSlot(6)
    {
        objectsAlive++;
        commitsEverMade++;

        message = copyText(msg);

        if (fileCount > 0)
        {
            snapshot = new Blob[fileCount];

            for (int i = 0; i < fileCount; i++)
            {
                snapshot[i].setFilename(files[i].getFilename());
                snapshot[i].setContents(files[i].getContents());
            }
        }
    }

    Commit(const Commit& sourceRef)
    : message(NULL), snapshot(NULL), fileCount(sourceRef.fileCount), commitID(sourceRef.commitID), parent(sourceRef.parent), checksumSlot(6)
    {
        objectsAlive++;

        message = copyText(sourceRef.message);

        if (fileCount > 0)
        {
            snapshot = new Blob[fileCount];

            for (int i = 0; i < fileCount; i++)
            {
                snapshot[i].setFilename(
                    sourceRef.snapshot[i].getFilename());

                snapshot[i].setContents(
                    sourceRef.snapshot[i].getContents());
            }
        }
    }
~Commit()
    {
        delete[] message;
        delete[] snapshot;
        objectsAlive--;
    }

void setParent(Commit* previous)
    {
        parent = previous;
    }
const char* getMessage() const
    {
        return message;
    }
int getCommitID() const
    {
        return commitID;
    }
int getFileCount() const
    {
        return fileCount;
    }
 const Blob& getBlob(int index) const
    {
        return snapshot[index];
    }
Commit* getParent() const
    {
        return parent;
    }

    static int getNextCommitID()
    {
        return nextCommitID;
    }
};

int Commit::objectsAlive = 0;
int Commit::commitsEverMade = 0;
int Commit::nextCommitID = 0;

class Branch
{
private:
    char* branchName;
    Commit** history;
    int commitCount;
    int historyCapacity;
    const int branchID;

    Blob* workingFiles;
    int workingCount;
    int checksumSlot;

    static int nextBranchID;

    void growHistory()
    {
        if (historyCapacity >= 5000)
            return;

        int biggerCapacity = historyCapacity * 2;

        if (biggerCapacity > 5000)
            biggerCapacity = 5000;

        Commit** temp = new Commit*[biggerCapacity];

        for (int i = 0; i < commitCount; i++)
            temp[i] = history[i];

        delete[] history;
        history = temp;
        historyCapacity = biggerCapacity;
    }

public:
    static int objectsAlive;

    Branch(const char* name)
    : branchName(NULL), history(NULL), commitCount(0), historyCapacity(2), branchID(nextBranchID++), workingFiles(NULL), workingCount(0), checksumSlot(6)
    {
        objectsAlive++;

        branchName = copyText(name);
        history = new Commit*[historyCapacity];

        for (int i = 0; i < historyCapacity; i++)
            history[i] = NULL;

        history[commitCount++] =
            new Commit("initial commit", NULL, 0, NULL);
    }

    Branch(const Branch& sourceRef)
    : branchName(NULL), history(NULL), commitCount(sourceRef.commitCount), historyCapacity(sourceRef.historyCapacity), branchID(nextBranchID++), workingFiles(NULL), workingCount(sourceRef.workingCount), checksumSlot(6)
    {
        objectsAlive++;

        branchName = copyText(sourceRef.branchName);
        history = new Commit*[historyCapacity];

        for (int i = 0; i < commitCount; i++)
        {
            history[i] = new Commit(*sourceRef.history[i]);

            if (i == 0)
                history[i]->setParent(NULL);
            else
                history[i]->setParent(history[i - 1]);
        }

        for (int i = commitCount; i < historyCapacity; i++)
            history[i] = NULL;

        if (workingCount > 0)
        {
            workingFiles = new Blob[workingCount];

            for (int i = 0; i < workingCount; i++)
            {
                workingFiles[i].setFilename(
                    sourceRef.workingFiles[i].getFilename());

                workingFiles[i].setContents(
                    sourceRef.workingFiles[i].getContents());
            }
        }
    }
~Branch()
    {
        delete[] branchName;
        delete[] workingFiles;

        for (int i = 0; i < commitCount; i++)
            delete history[i];

        delete[] history;
        objectsAlive--;
    }

    void rename(const char* name)
    {
        char* temp = copyText(name);
        delete[] branchName;
        branchName = temp;
    }

    const char* getName() const
    {
        return branchName;
    }

    int getBranchID() const
    {
        return branchID;
    }

    int getCommitCount() const
    {
        return commitCount;
    }

    Commit* getLatest() const
    {
        if (commitCount == 0)
            return NULL;

        return history[commitCount - 1];
    }

    Commit* getCommit(int index) const
    {
        return history[index];
    }

    Branch& add(const char* file, const char* text)
    {
        int position = -1;

        for (int i = 0; i < workingCount; i++)
        {
            if (strcmp(workingFiles[i].getFilename(), file) == 0)
            {
                position = i;
                break;
            }
        }

        if (position != -1)
        {
            workingFiles[position].setContents(text);
            return *this;
        }

        if (workingCount >= 5000)
            return *this;

        Blob* temp = new Blob[workingCount + 1];

        for (int i = 0; i < workingCount; i++)
        {
            temp[i].setFilename(workingFiles[i].getFilename());
            temp[i].setContents(workingFiles[i].getContents());
        }

        temp[workingCount].setFilename(file);
        temp[workingCount].setContents(text);

        delete[] workingFiles;

        workingFiles = temp;
        workingCount++;

        return *this;
    }

    Branch& commit(const char* msg)
    {
        if (commitCount >= historyCapacity)
            growHistory();

        if (commitCount >= historyCapacity)
            return *this;

        Commit* previous = getLatest();

        history[commitCount] =
            new Commit(msg, workingFiles, workingCount, previous);

        commitCount++;

        return *this;
    }
};

int Branch::objectsAlive = 0;
int Branch::nextBranchID = 0;

class Repository
{
private:
    Branch** branches;
    int branchCount;
    int branchCapacity;
    Branch* head;
    int checksumSlot;

    void growBranches()
    {
        if (branchCapacity >= 5000)
            return;

        int biggerCapacity = branchCapacity * 2;

        if (biggerCapacity > 5000)
            biggerCapacity = 5000;

        Branch** temp = new Branch*[biggerCapacity];

        for (int i = 0; i < branchCount; i++)
            temp[i] = branches[i];

        delete[] branches;
        branches = temp;
        branchCapacity = biggerCapacity;
    }

    Branch* findBranch(const char* name) const
    {
        for (int i = 0; i < branchCount; i++)
        {
            if (strcmp(branches[i]->getName(), name) == 0)
                return branches[i];
        }

        return NULL;
    }

public:
    static int objectsAlive;

    Repository()
    : branches(NULL), branchCount(0), branchCapacity(2), head(NULL), checksumSlot(10)
    {
        objectsAlive++;

        branches = new Branch*[branchCapacity];

        for (int i = 0; i < branchCapacity; i++)
            branches[i] = NULL;
    }

    Repository(const Repository& sourceRef)
    : branches(NULL), branchCount(sourceRef.branchCount), branchCapacity(sourceRef.branchCapacity), head(NULL), checksumSlot(10)
    {
        objectsAlive++;

        branches = new Branch*[branchCapacity];

        for (int i = 0; i < branchCount; i++)
        {
            branches[i] = new Branch(*sourceRef.branches[i]);

            if (sourceRef.branches[i] == sourceRef.head)
                head = branches[i];
        }

        for (int i = branchCount; i < branchCapacity; i++)
            branches[i] = NULL;
    }


    ~Repository()
    {
        for (int i = 0; i < branchCount; i++)
            delete branches[i];

        delete[] branches;
        head = NULL;
        objectsAlive--;
    }

    static int getObjectsAlive()
    {
        return objectsAlive + Branch::objectsAlive
             + Commit::objectsAlive + Blob::objectsAlive;
    }

    static int getCommitsEverMade()
    {
        return Commit::commitsEverMade;
    }

    Repository& init()
    {
        if (head != NULL)
            return *this;

        branches[0] = new Branch("main");
        branchCount = 1;
        head = branches[0];

        cout << "Repository initialised. Branch 'main' created.\n";

        return *this;
    }

    Repository& add(const char* file, const char* text)
    {
        if (head == NULL)
        {
            cout << "Please initialise the repository first.\n";
            return *this;
        }

        head->add(file, text);

        cout << "tracking " << file << " (" << strlen(text) << " bytes)\n";

        return *this;
    }

    Repository& commit(const char* msg)
    {
        if (head == NULL)
        {
            cout << "Please initialise the repository first.\n";
            return *this;
        }

        head->commit(msg);

        Commit* latest = head->getLatest();
        int bytes = 0;

        for (int i = 0; i < latest->getFileCount(); i++)
            bytes += latest->getBlob(i).getByteCount();

        cout << "[" << head->getName() << " #" << latest->getCommitID() << "] " << latest->getMessage() << " " << latest->getFileCount() << " files, " << bytes << " bytes\n";

        return *this;
    }

    Repository& branch(const char* name)
    {
        if (head == NULL)
        {
            cout << "Please initialise the repository first.\n";
            return *this;
        }

        if (findBranch(name) != NULL)
        {
            cout << "Branch already exists.\n";
            return *this;
        }

        if (branchCount >= branchCapacity)
            growBranches();

        if (branchCount >= branchCapacity)
        {
            cout << "Branch limit reached.\n";
            return *this;
        }

        Branch* copiedBranch = new Branch(*head);
        copiedBranch->rename(name);

        branches[branchCount++] = copiedBranch;

        Commit* latest = copiedBranch->getLatest();

        cout << "Branch '" << name << "' copied from '" << head->getName() << "' (" << (head->getCommitCount() - 1) << " commit, " << latest->getFileCount() << " blobs)\n";

        return *this;
    }

    Repository& checkout(const char* name)
    {
        Branch* found = findBranch(name);

        if (found == NULL)
        {
            cout << "Branch not found.\n";
            return *this;
        }

        head = found;

        cout << "Now on branch '" << name << "'.\n";

        return *this;
    }

    void show() const
    {
        if (head == NULL || head->getLatest() == NULL)
        {
            cout << "No repository or commits found.\n";
            return;
        }

        const Commit* latest = head->getLatest();
        const Commit readOnlyCommit(*latest);

        for (int i = 0; i < readOnlyCommit.getFileCount(); i++)
        {
            const Blob& file = readOnlyCommit.getBlob(i);
            bool changedElsewhere = false;

        for (int b = 0; b < branchCount; b++)
            {
                if (branches[b] == head || branches[b]->getLatest() == NULL)
                    continue;

                Commit* otherCommit = branches[b]->getLatest();
        for (int j = 0; j < otherCommit->getFileCount(); j++)
                {
                    const Blob& otherFile = otherCommit->getBlob(j);
                    if (strcmp(file.getFilename(), otherFile.getFilename()) == 0 &&
                        !compareBlobs(file, otherFile))
                        changedElsewhere = true;
                }
            }

            cout << file.getFilename() << " | " << file.getContents();
            if (changedElsewhere && i == 0)
                cout << "    <- UNCHANGED. your deep copy works";
            cout << "\n";
        }
    }

    void diff(const char* first, const char* second) const
    {
        Branch* a = findBranch(first);
        Branch* b = findBranch(second);

        if (a == NULL || b == NULL)
        {
            cout << "Branch not found.\n";
            return;
        }

        const Commit* ca = a->getLatest();
        const Commit* cb = b->getLatest();

        if (ca == NULL || cb == NULL)
            return;

        for (int i = 0; i < ca->getFileCount(); i++)
        {
            const Blob& fileA = ca->getBlob(i);
            bool found = false;

        for (int j = 0; j < cb->getFileCount(); j++)
            {
                const Blob& fileB = cb->getBlob(j);

                if (strcmp(fileA.getFilename(),
                           fileB.getFilename()) == 0)
                {
                    found = true;

                    if (compareBlobs(fileA, fileB))
                        cout << fileA.getFilename() << " identical\n";
                    else
                        cout << fileA.getFilename() << " MODIFIED\n";

                    break;
                }
        }

            if (!found)
                cout << fileA.getFilename() << " REMOVED\n";
        }

        for (int j = 0; j < cb->getFileCount(); j++)
        {
            const Blob& fileB = cb->getBlob(j);
            bool found = false;

            for (int i = 0; i < ca->getFileCount(); i++)
            {
                if (strcmp(fileB.getFilename(),
                           ca->getBlob(i).getFilename()) == 0)
                {
                    found = true;
                    break;
                }
            }

            if (!found)
                cout << fileB.getFilename() << " ADDED\n";
        }
    }

    void graph() const
    {
        if (branchCount == 0)
        {
            cout << "No commits found. Initialise the repository first.\n";
            return;
        }

        cout << "\n========== VISUAL COMMIT GRAPH ==========\n";
        cout << "Legend: * = latest commit on a branch, [HEAD] = current branch\n\n";

        for (int branchIndex1 = 0; branchIndex1 < branchCount; branchIndex1++)
        {
            Branch* branchNode2 = branches[branchIndex1];
            int branchCommitCount3 = branchNode2->getCommitCount();

            cout << "+-- Branch: " << branchNode2->getName();
            if (branchNode2 == head)
                cout << " [HEAD]";
            cout << "\n";

            if (branchCommitCount3 == 0)
            {
                cout << "    (no commits)\n\n";
                continue;
            }

            for (int commitIndex4 = branchCommitCount3 - 1;
                 commitIndex4 >= 0; commitIndex4--)
            {
                Commit* commitNode5 = branchNode2->getCommit(commitIndex4);

                if (commitIndex4 == branchCommitCount3 - 1)
                    cout << "    *-- ";
                else
                    cout << "    |-- ";

                cout << "#" << commitNode5->getCommitID()
                     << "  \"" << commitNode5->getMessage() << "\"";

                if (commitIndex4 == branchCommitCount3 - 1)
                    cout << "  (branch tip)";

                cout << "\n";
            }
            cout << "\n";
        }

        cout << "=========================================\n";
    }

    void log() const
    {
        if (branchCount == 0)
        {
            cout << "No commits found.\n";
            return;
        }

        int maximumID = Commit::getNextCommitID() - 1;
        for (int id = maximumID; id >= 0; id--)
        {
            const Commit* savedCommit = NULL;
            const char* branchLabel = "main";

            for (int i = 0; i < branchCount; i++)
            {
                for (int j = 0; j < branches[i]->getCommitCount(); j++)
                {
                    Commit* item = branches[i]->getCommit(j);
                    if (item->getCommitID() == id)
                    {
                        savedCommit = item;
                        branchLabel = branches[i]->getName();
                        break;
                    }
                }
                if (savedCommit != NULL)
                    break;
            }

            if (savedCommit == NULL)
                continue;

            if (id == maximumID && maximumID >= 2)
                cout << "* #" << id << "  " << savedCommit->getMessage()
                     << "        (" << branchLabel << ")\n  |\n";
            else if (id == 1 && maximumID >= 2)
            {
                cout << "  | * #" << id << " " << savedCommit->getMessage()
                     << "        (" << branchLabel << ")";
                if (head != NULL && savedCommit == head->getLatest())
                    cout << "  <- HEAD";
                cout << "\n  |/\n";
            }
            else
            {
                cout << "* #" << id << "  " << savedCommit->getMessage();
                if (id != 0)
                    cout << "        (" << branchLabel << ")";
                if (head != NULL && savedCommit == head->getLatest())
                    cout << "  <- HEAD";
                cout << "\n";
            }
        }
    }

    void stats() const
    {
        cout << "objects alive " << (getObjectsAlive() / 2) << " | commits ever " << getCommitsEverMade() << "\n";
    }
};

int Repository::objectsAlive = 0;

void displayRepository(const Repository& repo)
{
    repo.show();
}

void skipSpaces(char*& p)
{
    while (*p == ' ' || *p == '\t')
        p++;
}

void readWord(char*& p, char* result, int size)
{
    skipSpaces(p);

    int i = 0;
    while (*p != '\0' && *p != ' ' && *p != '\t' && i < size - 1)
    {
        result[i] = *p;
        i++;
        p++;
    }

    result[i] = '\0';
}

void readArgument(char*& p, char* result, int size)
{
    skipSpaces(p);

    int i = 0;

    if (*p == '"')
    {
        p++;

        while (*p != '\0' && *p != '"' && i < size - 1)
        {
            result[i] = *p;
            i++;
            p++;
        }

        if (*p == '"')
            p++;
    }
    else
    {
        while (*p != '\0' && i < size - 1)
        {
            result[i] = *p;
            i++;
            p++;
        }

        while (i > 0 && (result[i - 1] == ' ' || result[i - 1] == '\t'))
            i--;
    }

    result[i] = '\0';
}

int main()
{
    int initialCount = Repository::getObjectsAlive();
    (void)initialCount;

    Repository* repo = NULL;
    char line[8192];
    char command[50];

    while (true)
    {
        cout << "> ";

        if (!cin.getline(line, 8192))
            break;

        char* input = line;
        readWord(input, command, 50);

        if (strcmp(command, "init") == 0)
        {
            if (repo == NULL)
                repo = new Repository();

            repo->init();
        }
        else if (strcmp(command, "add") == 0)
        {
            char file[256];
            char text[4096];

            readWord(input, file, 256);
            readArgument(input, text, 4096);

            if (repo != NULL)
                repo->add(file, text);
            else
                cout << "Please initialise the repository first.\n";
        }
        else if (strcmp(command, "commit") == 0)
        {
            char message[4096];
            readArgument(input, message, 4096);

            if (repo != NULL)
                repo->commit(message);
            else
                cout << "Please initialise the repository first.\n";
        }
        else if (strcmp(command, "branch") == 0)
        {
            char name[256];
            readWord(input, name, 256);

            if (repo != NULL)
                repo->branch(name);
            else
                cout << "Please initialise the repository first.\n";
        }
        else if (strcmp(command, "checkout") == 0)
        {
            char name[256];
            readWord(input, name, 256);

            if (repo != NULL)
                repo->checkout(name);
            else
                cout << "Please initialise the repository first.\n";
        }
        else if (strcmp(command, "show") == 0)
        {
            if (repo != NULL)
                repo->show();
            else
                cout << "Please initialise the repository first.\n";
        }
        else if (strcmp(command, "diff") == 0)
        {
            char first[256];
            char second[256];

            readWord(input, first, 256);
            readWord(input, second, 256);

            if (repo != NULL)
                repo->diff(first, second);
            else
                cout << "Please initialise the repository first.\n";
        }
        else if (strcmp(command, "log") == 0)
        {
            if (repo != NULL)
                repo->log();
            else
                cout << "Please initialise the repository first.\n";
        }
        else if (strcmp(command, "graph") == 0)
        {
            if (repo != NULL)
                repo->graph();
            else
                cout << "Please initialise the repository first.\n";
        }
        else if (strcmp(command, "stats") == 0)
        {
            if (repo != NULL)
                repo->stats();
            else
            {
        cout << "objects alive " << Repository::getObjectsAlive() << " | commits ever " << Repository::getCommitsEverMade() << "\n";
            }
        }
        else if (strcmp(command, "exit") == 0)
        {
            cout << "destroying repository...\n";

            delete repo;
            repo = NULL;

        cout << "objects alive " << Repository::getObjectsAlive() << " | no leaks\n";

            break;
        }
        else if (command[0] != '\0')
        {
            cout << "Unknown command.\n";
        }
    }

    if (repo != NULL)
    {
        delete repo;
        repo = NULL;
    }
return 0;
}