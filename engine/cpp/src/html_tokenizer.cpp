#include "lapislazuli/html_tokenizer.hpp"
#include <cctype>
#include <string>
#include <utility>
namespace lapislazuli::html {
namespace {
bool ws(char c){return std::isspace(static_cast<unsigned char>(c))!=0;}
bool nameEnd(char c){return ws(c)||c=='/'||c=='>';}
std::size_t tagEnd(std::string_view s,std::size_t p){
 char q='\0'; for(std::size_t i=p;i<s.size();++i){char c=s[i];if(q){if(c==q)q='\0';}else if(c=='"'||c=='\'')q=c;else if(c=='>')return i;}return std::string_view::npos;
}
std::string lower(std::string s){for(char&c:s)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));return s;}
void attrs(std::string_view s,std::size_t p,Token&t){
 while(p<s.size()){
  while(p<s.size()&&ws(s[p]))++p;if(p>=s.size())break;
  if(s[p]=='/'){t.self_closing=true;++p;continue;}
  auto b=p;while(p<s.size()&&!nameEnd(s[p])&&s[p]!='=')++p;
  if(p==b){++p;continue;}
  auto n=lower(std::string(s.substr(b,p-b)));while(p<s.size()&&ws(s[p]))++p;
  std::string v;
  if(p<s.size()&&s[p]=='='){
   ++p;while(p<s.size()&&ws(s[p]))++p;
   if(p<s.size()&&(s[p]=='"'||s[p]=='\'')){char q=s[p++];auto x=p;while(p<s.size()&&s[p]!=q)++p;v=std::string(s.substr(x,p-x));if(p<s.size())++p;}
   else{auto x=p;while(p<s.size()&&!ws(s[p]))++p;v=std::string(s.substr(x,p-x));}
  }
  t.attributes.push_back({std::move(n),std::move(v)});
 }
}
}
std::vector<Token> Tokenizer::tokenize(std::string_view s)const{
 std::vector<Token> out;std::size_t p=0;
 while(p<s.size()){
  if(s.substr(p,4)=="<!--"){auto e=s.find("-->",p+4);auto x=e==std::string_view::npos?s.size():e;out.push_back({TokenType::Comment,{},std::string(s.substr(p+4,x-p-4)),{},false});p=e==std::string_view::npos?s.size():e+3;continue;}
  if(s[p]=='<'){
   auto e=tagEnd(s,p+1);if(e==std::string_view::npos){out.push_back({TokenType::Text,{},std::string(s.substr(p)),{},false});break;}
   auto in=s.substr(p+1,e-p-1);std::size_t i=0;while(i<in.size()&&ws(in[i]))++i;
   if(i<in.size()&&(in[i]=='!'||in[i]=='?')){p=e+1;continue;}
   bool close=false;if(i<in.size()&&in[i]=='/'){close=true;++i;while(i<in.size()&&ws(in[i]))++i;}
   auto b=i;while(i<in.size()&&!nameEnd(in[i]))++i;
   if(i==b){out.push_back({TokenType::Text,{},std::string(s.substr(p,e-p+1)),{},false});p=e+1;continue;}
   Token t{close?TokenType::EndTag:TokenType::StartTag,lower(std::string(in.substr(b,i-b))),{}, {},false};
   if(!close)attrs(in,i,t);out.push_back(std::move(t));p=e+1;continue;
  }
  auto e=s.find('<',p);auto x=e==std::string_view::npos?s.size():e;out.push_back({TokenType::Text,{},std::string(s.substr(p,x-p)),{},false});p=x;
 }
 return out;
}
}
