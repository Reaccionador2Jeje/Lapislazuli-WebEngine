#include "lapislazuli/html_tokenizer.hpp"
#include <cassert>
#include <iostream>
int main(){
 using namespace lapislazuli::html; Tokenizer z;
 auto t=z.tokenize("<html lang='en' data-note=hello disabled class=\"wide box\">Hello</html><!--note-->");
 assert(t.size()==4&&t[0].type==TokenType::StartTag&&t[0].name=="html");
 assert(t[0].attributes.size()==4&&t[0].attributes[0].value=="en");
 assert(t[0].attributes[1].value=="hello"&&t[0].attributes[2].name=="disabled");
 assert(t[0].attributes[3].value=="wide box"&&t[1].data=="Hello");
 assert(t[2].type==TokenType::EndTag&&t[3].type==TokenType::Comment);
 auto q=z.tokenize("<p title=\"a > b\">x</p>");
 assert(q.size()==3&&q[0].attributes[0].value=="a > b");
 auto sc=z.tokenize("<img src='x' />"); assert(sc.size()==1&&sc[0].self_closing);
 std::cout<<"HTML tokenizer tests passed.\n";
}
