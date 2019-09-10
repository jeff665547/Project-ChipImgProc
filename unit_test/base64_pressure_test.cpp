#include <iostream>
#include <fstream>
#include <string>
int main() {
    std::ofstream out("test.html");
    std::string image = "/9j/4AAQSkZJRgABAQAAAQABAAD/2wBDAAIBAQEBAQIBAQECAgICAgQDAgICAgUEBAMEBgUGBgYFBgYGBwkIBgcJBwYGCAsICQoKCgoKBggLDAsKDAkKCgr/wAALCAARABEBAREA/8QAHwAAAQUBAQEBAQEAAAAAAAAAAAECAwQFBgcICQoL/8QAtRAAAgEDAwIEAwUFBAQAAAF9AQIDAAQRBRIhMUEGE1FhByJxFDKBkaEII0KxwRVS0fAkM2JyggkKFhcYGRolJicoKSo0NTY3ODk6Q0RFRkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqDhIWGh4iJipKTlJWWl5iZmqKjpKWmp6ipqrKztLW2t7i5usLDxMXGx8jJytLT1NXW19jZ2uHi4+Tl5ufo6erx8vP09fb3+Pn6/9oACAEBAAA/AP3b1LUdEtL62mv9QBlhnEcasrTuCVIIVUJxIWOMkdOAOadbWthPIEjume0BWOMXDgj5zloyz7jLuyBjtjGc9NP/AIlX/QOP/gE3/wATWcIL1dRNpqERnwkbRu0eQrKRmXcRtDDGQNuc8ZxVy4torhXubJLR5ATFJMUDMORuyQVxgjlc9R61c+xxf35f+/7/AONQn/kLD/dP8hVtuh+lcvX/2Q==";
    out << "<html>" << std::endl;
    auto nums = 25 * 36;
    for(int i = 0; i < nums; i ++ ) {
        out << "<div>";
        out << "<img src='data:image/png;base64,";
        out << image << "' />";
        out << "</div>";
    }
    out << "</html>" << std::endl;
}