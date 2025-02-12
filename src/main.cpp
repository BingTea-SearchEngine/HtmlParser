#include <iostream>

#include "Parser.hpp"

using std::cout, std::endl;

int main() {
    std::string html = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <title>Test HTML Page</title>
            <meta charset="UTF-8">
            <meta name="description" content="Sample HTML for Parser Testing">
            <style>
                body { font-family: Arial, sans-serif; }
                .highlight { color: red; }
            </style>
        </head>
        <body>
            <h1>Welcome to the Test Page</h1>
            <p>This is a <strong>bold</strong> and <em>italic</em> paragraph.</p>
            <a href="https://example.com">Click here</a> to visit example.com.
            
            <ul>
                <li>Item 1</li>
                <li class="highlight">Item 2</li>
                <li>Item 3</li>
            </ul>

            <table border="1">
                <tr>
                    <th>Name</th>
                    <th>Age</th>
                </tr>
                <tr>
                    <td>Alice</td>
                    <td>30</td>
                </tr>
                <tr>
                    <td>Bob</td>
                    <td>25</td>
                </tr>
            </table>

            <div id="footer">Footer content here</div>
            <script>
                console.log("JavaScript in HTML test");
            </script>
        </body>
        </html>
    )";

    Parser parser = Parser(html);

    cout << "Title words:" << endl;
    for (auto word : parser.getTitle()) {
        cout << word << " ";
    }
    cout << endl;

    cout << "Words:" << endl;
    for (auto word : parser.getWords()) {
        cout << word << " ";
    }
    cout << endl;

    cout << "Urls:" << endl;
    for (auto url : parser.getUrls()) {
        cout << url.url << ": ";
        for (auto word : url.anchorText) {
            cout << word << " ";
        }
    }
    cout << endl;
}
