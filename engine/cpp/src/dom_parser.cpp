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
bool isVoid(std::string_view n){static constexpr std::string_view a[]={"area","base","br","col","embed","hr","img","input","link","meta","param","source","track","wbr"};return std::find(std::begin(a),std::end(a),n)!=std::end(a);}
void utf8(std::string&o,std::uint32_t c){if(c<=127)o.push_back(char(c));else if(c<=2047){o.push_back(char(192|(c>>6)));o.push_back(char(128|(c&63)));}else if(c<=65535&&!(c>=55296&&c<=57343)){o.push_back(char(224|(c>>12)));o.push_back(char(128|((c>>6)&63)));o.push_back(char(128|(c&63)));}else if(c<=1114111){o.push_back(char(240|(c>>18)));o.push_back(char(128|((c>>12)&63)));o.push_back(char(128|((c>>6)&63)));o.push_back(char(128|(c&63)));}else utf8(o,65533);}
int hx(char c){if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;if(c>='A'&&c<='F')return c-'A'+10;return -1;}
std::string decode(std::string_view s){std::string o;o.reserve(s.size());for(std::size_t i=0;i<s.size();){if(s[i]!='&'){o.push_back(s[i++]);continue;}auto e=s.find(';',i+1);if(e==s.npos||e-i>16){o.push_back(s[i++]);continue;}auto n=s.substr(i+1,e-i-1);std::uint32_t c=0;bool ok=true;if(n.empty())ok=false;else if(n[0]=='#'){std::size_t p=1;int base=10;if(p<n.size()&&(n[p]=='x'||n[p]=='X')){base=16;++p;}if(p==n.size())ok=false;for(;ok&&p<n.size();++p){int v=base==16?hx(n[p]):(n[p]>='0'&&n[p]<='9'?n[p]-'0':-1);if(v<0||v>=base||c>(0x10ffffu-unsigned(v))/unsigned(base)){ok=false;break;}c=c*unsigned(base)+unsigned(v);}if(c==0||c>0x10ffff||(c>=0xd800&&c<=0xdfff))c=65533;}else if(n=="amp")c='&';else if(n=="lt")c='<';else if(n=="gt")c='>';else if(n=="quot")c='"';else if(n=="apos")c='\'';else if(n=="nbsp")c=160;else ok=false;if(ok)utf8(o,c);else o.append(s.substr(i,e-i+1));i=e+1;}return o;}
}
std::unique_ptr<dom::Document> DocumentParser::parse(std::string_view source)const{
 Tokenizer z;auto ts=z.tokenize(source);auto d=std::make_unique<dom::Document>();std::vector<dom::Node*> st{d.get()};
 for(const auto&t:ts)switch(t.type){
 case TokenType::Text:if(!t.data.empty())st.back()->appendChild(d->createTextNode(decode(t.data)));break;
 case TokenType::Comment:st.back()->appendChild(d->createComment(t.data));break;
 case TokenType::StartTag:{auto e=d->createElement(t.name);for(const auto&a:t.attributes)if(!e->hasAttribute(a.name))e->setAttribute(a.name,decode(a.value));auto n=e->name();auto&ins=st.back()->appendChild(std::move(e));if(!isVoid(n))st.push_back(&ins);break;}
 case TokenType::EndTag:for(std::size_t i=st.size();i>1;--i)if(st[i-1]->name()==t.name){st.resize(i-1);break;}break;
 }
 return d;
}
}
