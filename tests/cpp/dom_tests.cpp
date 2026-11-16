#include "lapislazuli/dom.hpp"
#include "lapislazuli/dom_parser.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
using namespace lapislazuli::dom;
int main(){
 Document d; auto html=d.createElement("HTML");html->setAttribute("lang","en");
 auto body=d.createElement("body");body->setAttribute("id","main");
 auto p=d.createElement("p");p->appendChild(d.createTextNode("Hello <web>"));
 body->appendChild(std::move(p));html->appendChild(std::move(body));
 Element& root=static_cast<Element&>(d.appendChild(std::move(html)));
 assert(d.documentElement()==&root&&d.getElementById("main"));
 assert(d.getElementsByTagName("p").size()==1&&root.textContent()=="Hello <web>");
 assert(d.serialize()=="<html lang=\"en\"><body id=\"main\"><p>Hello &lt;web&gt;</p></body></html>");
 auto* b=d.getElementById("main");auto detached=root.removeChild(*b);
 assert(!detached->parent()&&!d.getElementById("main"));root.appendChild(std::move(detached));
 bool rejected=false;try{b->appendChild(nullptr);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
 lapislazuli::html::DocumentParser parser;
 auto parsed=parser.parse("<!DOCTYPE html><html lang='es' data-info=\"A &amp; B\"><body><img src='logo.png' alt=Logo disabled><p id='greeting' class='big'>Hola &amp; bienvenido &#33; &#x1F30E; &unknown;</p></body></html>");
 auto* he=parsed->documentElement();assert(he&&*he->getAttribute("lang")=="es");
 assert(*he->getAttribute("data-info")=="A & B");
 auto* img=parsed->getElementsByTagName("img").front();
 assert(*img->getAttribute("src")=="logo.png"&&*img->getAttribute("alt")=="Logo"&&img->hasAttribute("disabled"));
 auto* greet=parsed->getElementById("greeting");
 assert(greet&&*greet->getAttribute("class")=="big");
 assert(greet->textContent()=="Hola & bienvenido ! 🌎 &unknown;");
 assert(parsed->getElementsByTagName("p").size()==1&&parsed->getElementsByTagName("*").size()==4);
 assert(parsed->serialize().find("data-info=\"A &amp; B\"")!=std::string::npos);
 std::cout<<"DOM and HTML parser tests passed.\n";
}
