#include "lapislazuli/html_tokenizer.hpp"
#include <cctype>
#include <string>
#include <utility>
namespace lapislazuli::html {
namespace {
bool isSpace(char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; }
bool isNameEnd(char c) { return isSpace(c) || c == '/' || c == '>'; }
std::size_t findTagEnd(std::string_view s, std::size_t start) {
    char quote = '\0';
    for (std::size_t i=start;i<s.size();++i) {
        char c=s[i];
        if (quote) { if(c==quote) quote='\0'; }
        else if(c=='"' || c=='\'') quote=c;
        else if(c=='>') return i;
    }
    return std::string_view::npos;
}
std::string lower(std::string v) {
    for(char& c:v) c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return v;
}
void parseAttributes(std::string_view s,std::size_t p,Token& t) {
    while(p<s.size()) {
        while(p<s.size() && isSpace(s[p])) ++p;
        if(p>=s.size()) break;
        if(s[p]=='/') { t.self_closing=true; ++p; continue; }
        auto start=p;
        while(p<s.size() && !isNameEnd(s[p]) && s[p]!='=') ++p;
        if(p==start) { ++p; continue; }
        std::string name=lower(std::string(s.substr(start,p-start)));
        while(p<s.size() && isSpace(s[p])) ++p;
        std::string value;
        if(p<s.size() && s[p]=='=') {
            ++p; while(p<s.size() && isSpace(s[p])) ++p;
            if(p<s.size() && (s[p]=='"' || s[p]=='\'')) {
                char q=s[p++]; auto v=p;
                while(p<s.size() && s[p]!=q) ++p;
                value=std::string(s.substr(v,p-v)); if(p<s.size()) ++p;
            } else {
                auto v=p; while(p<s.size() && !isSpace(s[p])) ++p;
                value=std::string(s.substr(v,p-v));
            }
        }
        t.attributes.push_back({std::move(name),std::move(value)});
    }
}
}
std::vector<Token> Tokenizer::tokenize(std::string_view input) const {
    std::vector<Token> out; std::size_t p=0;
    while(p<input.size()) {
        if(input.substr(p,4)=="<!--") {
            auto e=input.find("-->",p+4); auto ce=e==std::string_view::npos?input.size():e;
            out.push_back({TokenType::Comment,{},std::string(input.substr(p+4,ce-p-4)),{},false});
            p=e==std::string_view::npos?input.size():e+3; continue;
        }
        if(input[p]=='<') {
            auto e=findTagEnd(input,p+1);
            if(e==std::string_view::npos) { out.push_back({TokenType::Text,{},std::string(input.substr(p)),{},false}); break; }
            auto inside=input.substr(p+1,e-p-1); std::size_t i=0;
            while(i<inside.size() && isSpace(inside[i])) ++i;
            if(i<inside.size() && (inside[i]=='!' || inside[i]=='?')) { p=e+1; continue; }
            bool closing=false;
            if(i<inside.size() && inside[i]=='/') { closing=true; ++i; while(i<inside.size()&&isSpace(inside[i]))++i; }
            auto ns=i; while(i<inside.size()&&!isNameEnd(inside[i]))++i;
            if(i==ns) { out.push_back({TokenType::Text,{},std::string(input.substr(p,e-p+1)),{},false}); p=e+1; continue; }
            Token t{closing?TokenType::EndTag:TokenType::StartTag,lower(std::string(inside.substr(ns,i-ns))),{}, {},false};
            if(!closing) parseAttributes(inside,i,t);
            out.push_back(std::move(t)); p=e+1; continue;
        }
        auto e=input.find('<',p); auto te=e==std::string_view::npos?input.size():e;
        out.push_back({TokenType::Text,{},std::string(input.substr(p,te-p)),{},false}); p=te;
    }
    return out;
}
} // namespace lapislazuli::html
