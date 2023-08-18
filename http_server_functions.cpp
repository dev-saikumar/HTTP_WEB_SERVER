#include <bits/stdc++.h>
#include "http_server_functions.h"
#include <fcntl.h>
#include <sys/stat.h>

string get_file_data_as_string(string path)
{
    struct stat buff;
    stat(path.c_str(), &buff);
    int fd = open(path.c_str(), O_RDONLY);
    int size = buff.st_size;
    string res_string;
    res_string.resize(size);
    read(fd, (char *)res_string.data(), size);
    close(fd);
    return res_string;
}

HTTP_Response *create_response_object(HTTP_Response *res, string status_code, string message, string body)
{
    res->status_code = status_code;
    res->status_text = message;
    res->content_type = "text/html";
    res->content_length = to_string(body.size());
    res->body = body;
    return res;
}

vector<string> split(string str, char delimiter)
{
    vector<string> parts;
    int prev = 0, i = 0;
    while (i < str.size())
    {
        if (str[i] == delimiter)
        {
            if (prev != i)
                parts.push_back(str.substr(prev, i - prev));
            else
                parts.push_back("");
            prev = i + 1;
        }
        i++;
    }
    if (prev != i)
        parts.push_back(str.substr(prev, i - 1 - prev));
    else
        parts.push_back("");
    return parts;
}

HTTP_Request::HTTP_Request(string request)
{
    vector<string> req_vec = split(request, '\n');

    vector<string> line_split = split(req_vec[0], ' ');
    if (line_split.size() != 3)
    {
        this->err_in_req = true;
        return;
    }
    else
        err_in_req = false;
    this->method = line_split[0];
    this->url = line_split[1];
    this->HTTP_version = line_split[2];
}

string HTTP_Response::get_string()
{
    string res_str = "HTTP/1.0";
    res_str = res_str + " " + this->status_code + " " + this->status_text + "\n";
    res_str = res_str + "Date: fri, 07 Oct 2022 16:46:00 GMT\n";
    res_str = res_str + "Content-Type: " + this->content_type + "\n";
    res_str = res_str + "Content-Length: " + this->content_length + "\n";
    res_str = res_str + "Conection: closed\n\n";
    res_str = res_str + body;
    return res_str;
}

int check_exist_type(string path)
{
    struct stat buff;
    if (stat(path.c_str(), &buff) != 0)
    {
        return 0; // path does not exist
    }
    if (S_ISDIR(buff.st_mode))
    {
        return 1; // given path is a directory
    }
    else if (S_ISREG(buff.st_mode))
    {
        return 2; // given path is a regular file
    }
    else
    {
        return 0; // path does not exist
    }
    return true;
}

HTTP_Response *handle_request(string request)
{
    HTTP_Request *req = new HTTP_Request(request);
    string err_path = "./html_files/err.html";
    HTTP_Response *res = new HTTP_Response();
    res->HTTP_version = req->HTTP_version;
    string path;
    if (req->err_in_req)
        return create_response_object(res, "404", "Invalid Request", get_file_data_as_string(err_path));
    if (req->method != "GET")
        return create_response_object(res, "404", "Invalid Request", get_file_data_as_string(err_path));


    switch (check_exist_type("html_files" + req->url))
    {
    case 0:
        return create_response_object(res, "404", "Invalid Request", get_file_data_as_string(err_path));
    case 1:
        if (check_exist_type("html_files" + req->url + "/index.html") == 2)
            path = "html_files" + req->url + "/index.html";
        else
            return create_response_object(res, "404", "Invalid Request", get_file_data_as_string(err_path));
        break;
    case 2:
        path = "./html_files" + req->url;
    }

    
    delete req;
    return create_response_object(res, "200", "OK", get_file_data_as_string(path));
}
