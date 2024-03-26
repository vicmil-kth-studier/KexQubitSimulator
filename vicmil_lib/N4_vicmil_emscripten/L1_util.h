#include "../N3_vicmil_opengl/vicmil_opengl.h"

#include <emscripten.h>

/**
 * Create help functions for interacting with javascript
*/
namespace vicmil {
    namespace browser {
        // The actual screen width and height
        static double window_width = 1000;
        static double window_height = 1000;
        
        void example_alert() {
            emscripten_run_script("alert('hi')");
        }

        EM_JS(void, _alert, (const char *alert_str, int len_alert_str), {
            alert(UTF8ToString(alert_str, len_alert_str));
        });
        void alert(std::string text) {
            _alert(text.c_str(), text.size());
        }

        EM_JS(void, _download_file, (const char *filename_ptr, int len_filename, const char *file_contents_ptr, int len_file_contents), {
            const filename = UTF8ToString(filename_ptr, len_filename);
            const file_contents = UTF8ToString(file_contents_ptr, len_file_contents);
            const file = new File([file_contents], filename, {
                type: 'text/plain',
            });

            const link = document.createElement('a');
            const url = URL.createObjectURL(file);

            link.href = url;
            link.download = file.name;

            document.body.appendChild(link);
            link.click();

            document.body.removeChild(link);
            window.URL.revokeObjectURL(url);
        });
        // May not work for any file contents(for now). Has to be representable as strings, so no 0 characters
        void download_file(std::string filename, std::vector<char> file_contents) {
            _download_file(filename.c_str(), filename.size(), &file_contents[0], file_contents.size());
        }
        void download_text_file(std::string filename, std::string file_contents) {
            std::vector<char> v(file_contents.begin(), file_contents.end());
            vicmil::browser::download_file(filename, v);
        }
    }
}


// Make sure javascript can send updates when screen size changes
extern "C" {
    int set_screen_size(double width, double height) {
        vicmil::browser::window_width = width;
        vicmil::browser::window_height = height;
        return 0;
    }
}