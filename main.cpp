#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "httplib.h"

using namespace std;


class Post {
public:
	string content;
	Post() = default;
	Post(const string& content) : content(content) {};
};


class Server {
private:
	size_t currId = 0;	
	map<size_t, Post> posts;
	httplib::Server svr;
	string fillVariables(const string& content, const unordered_map<string, string>& variables) {
		string newContent = content; 
		for (const pair<string, string>& var : variables) {
			size_t pos = newContent.find(var.first);
			while (pos != string::npos) {
				newContent.replace(pos, var.first.length(), var.second) ;
  				pos = newContent.find(var.first, pos + var.first.length());
			}
		}
		return newContent;
	}
	string getFile(const string& path, const unordered_map<string, string>& variables) {
		ifstream file(path);
		if (!file.is_open()) {
			return "404";
		}
		stringstream buffer;
		buffer << file.rdbuf();
		return fillVariables(buffer.str(), variables);
	}

public:
	Server(const string& address, const size_t& port) {
		svr.Get("/", [this](const httplib::Request& req, httplib::Response &res) {
			res.set_content("<script>window.location.href='/" + to_string(rand() % posts.size()) + "';</script>", "text/html");
		});
		svr.Get(R"(/(\d+))", [this](const httplib::Request& req, httplib::Response &res) {
			bool areTherePosts = bool(posts.size());
			res.set_content(getFile("html/index.html", {
				{"{postContent}", posts[stoi(req.matches[1])].content}			
			}), "text/html");
		});
		svr.Get("/upload", [this](const httplib::Request& req, httplib::Response &res) {
			if (req.has_param("text")) {
				Post post(string(req.get_param_value("text")));
				posts[currId] = post;
				currId++;
				cout << "Added '" << post.content << "'" << endl;
				res.set_content("<script>window.location.href='/';</script>", "text/html");
			} else {
				res.set_content(getFile("html/upload.html", {}), "text/html");
			}
		});
		svr.listen(address, port);
	}
};


int main() {
	srand(time(0));
	Server server("localhost", 8080);
	return 0;
}