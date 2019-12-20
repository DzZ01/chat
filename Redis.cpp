//
// Created by daimiaopeng on 2019/11/17.
//

#include "Redis.h"


auto Redis::redisReply_ptr(void *reply) {
    auto delRedisReply = [](redisReply *reply) {
        freeReplyObject(reply);
    };
    unique_ptr<redisReply, decltype(delRedisReply)> reply_unique_ptr(reinterpret_cast<redisReply *>(reply),
                                                                     delRedisReply);
    return reply_unique_ptr;
}

void Redis::pushMessageQueue(const string &message) {
    auto reply = redisReply_ptr(redisCommand(conn, "RPUSH messageQueue %s", message.c_str()));
    LOG(WARNING) << "添加到消息队列 :" << message;
}

int Redis::lenMessage() {
    string str = "LLEN messageQueue";
    auto reply = redisReply_ptr(redisCommand(conn, str.c_str()));
    int len = reply->integer;
    return len;
}

string Redis::popMessageQueue() {
    auto reply = redisReply_ptr(redisCommand(conn, "LPOP  messageQueue"));
    string str = reply->str;
    LOG(INFO) << "从消息队列取出：" << str;
    return str;
}


string Redis::getName(const string &token) {
    auto reply = redisReply_ptr(redisCommand(conn, "HMGET token %s", token.c_str()));
    string str;
    auto element = reply->element;
    if ((*element)->str == nullptr) {
        str = "";
        return str;
    }
    str = (*element)->str;
    LOG(INFO) << token + "(token->name)" << str;
    return str;
}


int Redis::getFd(const string &name) {
    auto reply = redisReply_ptr(redisCommand(conn, "HMGET fdName %s", name.c_str()));
    string str;
    auto element = reply->element;
    if ((*element)->str == nullptr) {
        str = "-1";
    } else {
        str = (*element)->str;
    }
    return stoi(str);
}

void Redis::setName(int fd, const string &name) {
    auto reply = redisReply_ptr(redisCommand(conn, "HMSET fdName %s %d", name.c_str(), fd));
}

string Redis::login(const string &name, const string &passwd) {
    auto reply = redisReply_ptr(redisCommand(conn, "HMGET login %s", name.c_str()));
    auto element = reply->element;
    if ((*element)->str == nullptr) {
        LOG(INFO) << name + " 该用户没有注册";
        return "";
    }
    string str = (*element)->str;
    if (passwd == str) {
        LOG(INFO) << name + " 登录成功";
        string token = name;
        setToken(token, name);
        return token;
    } else {
        LOG(INFO) << name + " 登录密码错误";
        return "";
    }
}

int Redis::registered(const string &name, const string &passwd) {
    auto reply = redisReply_ptr(redisCommand(conn, "HMGET login %s", name.c_str()));
    auto element = reply->element;
    if ((*element)->str != nullptr) {
        LOG(INFO) << name + " 该用户已注册";
        return 0;
    }
    redisCommand(conn, "HMSET login %s %s", name.c_str(), passwd.c_str());
    LOG(INFO) << name + " 注册成功";
    return 1;
}


void Redis::setToken(const string &token, const string &name) {
    auto reply = redisReply_ptr(redisCommand(conn, "HMSET token %s %s", token.c_str(), name.c_str()));
    LOG(INFO) << name + " (name->token)" << token;
}

void Redis::init() {

}

int Redis::getOnlineNums() {
    auto reply = redisReply_ptr(redisCommand(conn, "HLEN token"));
    LOG(INFO) << "当前token数："<<reply->integer;
    return reply->integer;
}

int Redis::getRegisterNums() {
    auto reply = redisReply_ptr(redisCommand(conn, "HLEN login"));
    LOG(INFO) << "当前注册人数："<<reply->integer;
    return reply->integer;
}



void Redis::clearToken() {
    auto reply = redisReply_ptr(redisCommand(conn, "DEL token"));
    if (reply->integer==1){
        LOG(INFO) <<"token 清空初始化成功";
    }else{
        LOG(ERROR) <<"token 清空初始化失败";
    }
}

