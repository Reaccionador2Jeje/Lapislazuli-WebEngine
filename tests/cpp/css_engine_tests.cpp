#include "lapislazuli/css/css_engine.hpp"
#include "lapislazuli/dom.hpp"
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>
int main(){
 using namespace lapislazuli;
 dom::Document d;auto html=d.createElement("html");auto body=d.createElement("body");body->setAttribute("class","page dark");
 auto card=d.createElement("div");card->setAttribute("id","hero");card->setAttribute("class","card featured");card->setAttribute("style","border: 1px solid black; color: purple");
 auto title=d.createElement("h1");title->setAttribute("class","title");card->appendChild(std::move(title));body->appendChild(std::move(card));html->appendChild(std::move(body));d.appendChild(std::move(html));
 auto*cardPtr=d.getElementById("hero");
 auto sheet=css::Parser{}.parse("body { display:block; color:black } .card { color:blue; padding:12px } #hero { color:red } .page > .card.featured { background:#eeeeee; margin:4px } .card .title, h1 { font-size:20px } .card { color:green !important }");
 assert(sheet.rules().size()==6);
 auto tokens=css::Tokenizer{}.tokenize("#hero.card > h1.title");assert(!tokens.empty());\n assert(tokens.front().type==css::TokenType::Hash&&tokens.front().text=="#hero");
 css::StyleResolver resolver;std::vector<const css::StyleSheet*> sheets{&sheet};auto style=resolver.compute(*cardPtr,sheets);
 assert(style.get("color")&&*style.get("color")=="green");
 assert(style.get("padding")&&*style.get("padding")=="12px");
 assert(style.get("background")&&*style.get("background")=="#eeeeee");
 assert(style.get("border")&&*style.get("border")=="1px solid black");
 auto tree=resolver.computeTree(d,sheets);assert(tree.size()==4);
 auto*heading=d.getElementsByTagName("h1").front();
 assert(tree.at(heading).get("font-size")&&*tree.at(heading).get("font-size")=="20px");
 std::cout<<"CSS tokenizer, parser, selector, and cascade tests passed.\n";
}
