#include "lapislazuli/dom_parser.hpp"
#include "lapislazuli/html_tokenizer.hpp"
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>
namespace lapislazuli::html {
namespace {
bool isVoid(std::string_view n) {
 static constexpr std::string_view tags[]={"area","base","br","col","embed","hr","img","input","link","meta","param","source","track","wbr"};
 return std::find(std::begin(tags),std::end(tags),n)!=std::end(tags);
}
void utf8(std::string& o,std::uint32_t c) {
 if(c<=0x7f)o.push_back(static_cast<char>(c));
 else if(c<=0x7ff){o.push_back(static_cast<char>(0xc0|(c>>6)));o.push_back(static_cast<char>(0x80|(c&63)));}
 else if(c<=0xffff && !(c>=0xd800&&c<=0xdfff)){o.push_back(static_cast<char>(0xe0|(c>>12)));o.push_back(static_cast<char>(0x80|((c>>6)&63)));o.push_back(static_cast<char>(0x80|(c&63)));}
 else if(c<=0x10ffff){o.push_back(static_cast<char>(0xf0|(c>>18)));o.push_back(static_cast<char>(0x80|((c>>12)&63)));o.push_back(static_cast<char>(0x80|((c>>6)&63)));o.push_back(static_cast<char>(0x80|(c&63)));}
 else utf8(o,0xfffd);
}
int hex(char c){if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;if(c>='A'&&c<='F')return c-'A'+10;return -1;}
std::string decode(std::string_view s) {
 std::string o; o.reserve(s.size());
 for(std::size_t i=0;i<s.size();) {
  if(s[i]!='&'){o.push_back(s[i++]);continue;}
  auto e=s.find(';',i+1); if(e==std::string_view::npos||e-i>16){o.push_back(s[i++]);continue;}
  auto n=s.substr(i+1,e-i-1); std::uint32_t cp=0; bool ok=true;
  if(n.empty())ok=false;
  else if(n[0]=='#') {
   std::size_t p=1; int base=10; if(p<n.size()&&(n[p]=='x'||n[p]=='X')){base=16;++p;}
   if(p==n.size())ok=false;
   for(;ok&&p<n.size();++p){int v=base==16?hex(n[p]):(n[p]>='0'&&n[p]<='9'?n[p]-'0':-1);
    if(v<0||v>=base||cp>(0x10ffffu-static_cast<unsigned>(v))/static_cast<unsigned>(base)){ok=false;break;}
    cp=cp*static_cast<unsigned>(base)+static_cast<unsigned>(v);}
   if(cp==0||cp>0x10ffff||(cp>=0xd800&&cp<=0xdfff))cp=0xfffd;
  } else if(n=="amp")cp='&'; else if(n=="lt")cp='<'; else if(n=="gt")cp='>';
  else if(n=="quot")cp='"'; else if(n=="apos")cp='\''; else if(n=="nbsp")cp=0xa0; else ok=false;
  if(ok)utf8(o,cp); else o.append(s.substr(i,e-i+1));
  i=e+1;
 }
 return o;
}
}
std::unique_ptr<dom::Document> DocumentParser::parse(std::string_view source) const {
 const Tokenizer tokenizer; const auto tokens=tokenizer.tokenize(source);
 auto doc=std::make_unique<dom::Document>(); std::vector<dom::Node*> stack{doc.get()};
 for(const Token& t:tokens) {
  switch(t.type) {
   case TokenType::Text:
    if(!t.data.empty())stack.back()->appendChild(doc->createTextNode(decode(t.data)));
    break;
   case TokenType::Comment: stack.back()->appendChild(doc->createComment(t.data)); break;
   case TokenType::StartTag: {
    auto el=doc->createElement(t.name);
    for(const auto& a:t.attributes) if(!el->hasAttribute(a.name))el->setAttribute(a.name,decode(a.value));
    auto name=el->name(); auto& inserted=stack.back()->appendChild(std::move(el));
    if(!t.self_closing&&!isVoid(name))stack.push_back(&inserted);
    break;
   }
   case TokenType::EndTag:
    for(std::size_t i=stack.size();i>1;--i)if(stack[i-1]->name()==t.name){stack.resize(i-1);break;}
    break;
  }
 }
 return doc;
}
} // namespace lapislazuli::html
