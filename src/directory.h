#include <iostream>
#include <filesystem>
#include <string>

using namespace std;
namespace fs = filesystem;

class Directory {
public:
    Directory(fs::path dir_name) : dir(dir_name), is_new(true) {
        if (!fs::exists(dir))
            fs::create_directory(dir);
        else {
            if (!fs::is_directory(dir))
                fs::create_directory(dir);
            else {
                cout << "found existed directory" << endl;
                is_new = false;
            }
        }
    }
    virtual ~Directory() {}
    inline fs::path path() {
        return dir;
    }
    inline string name() {
        return dir.string();
    }
    inline bool isNew() { return is_new; }

private:
    fs::path dir;
    bool is_new;
};

/* 
int main(int argc, char* argv[]) {
    
    Directory dirRoot("res");
    cout << "the root directory is: " << dirRoot.path() << endl;
    
    Directory dirStep1(dirRoot.path() / "step1");
    cout << "the step1 directory is: " << dirStep1.path() << endl;

    return 0;
} 
*/