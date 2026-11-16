#include "lapislazuli/dom.hpp"
#include "lapislazuli/dom_parser.hpp"
#include <cassert>
#include <iostream>
#include <stdexcept>
using namespace lapislazuli::dom;
int main(){Document d;auto h=d.createElement("HTML");h->setAttribute("lang","en");auto b=d.createElement("body");b->setAttribute("id","main");auto p=d.createElement("p");p->appendChild(d.createTextNode("Hello <web>"));b->appendChild(std::move(p));h->appendChild(std::move(b));Element&r=static_cast<Element&>(d.appendChild(std::move(h)));
assert(d.documentElement()==&r&&d.getElementById("main"));assert(d.getElementsByTagName("p").size()==1&&r.textContent()=="Hello <web>");
assert(d.serialize()=="<html lang=\"en\"><body id=\"main\"><p>Hello &lt;web&gt;</p></body></html>");
auto*bn=d.getElementById("main");auto detached=r.removeChild(*bn);assert(!detached->parent()&&!d.getElementById("main"));r.appendChild(std::move(detached));
bool rejected=false;try{bn->appendChild(nullptr);}catch(const std::invalid_argument&){rejected=true;}assert(rejected);
lapislazuli::html::DocumentParser z;auto x=z.parse("<!DOCTYPE html><html lang='es' data-info=\"A &amp; B\"><body><img src='logo.png' alt=Logo disabled><p id='greeting' class='big'>Hola &amp; bienvenido &#33; &#x1F30E; &unknown;</p></body></html>");
auto*he=x->documentElement();assert(he&&*he->getAttribute("lang")=="es"&&*he->getAttribute("data-info")=="A & B");
auto*img=x->getElementsByTagName("img").front();assert(*img->getAttribute("src")=="logo.png"&&*img->getAttribute("alt")=="Logo"&&img->hasAttribute("disabled"));
auto*g=x->getElementById("greeting");assert(g&&*g->getAttribute("class")=="big"&&g->textContent()=="Hola & bienvenido ! 🌎 &unknown;");
assert(x->getElementsByTagName("p").size()==1&&x->getElementsByTagName("*").size()==4);
assert(x->serialize().find("data-info=\"A &amp; B\"")!=std::string::npos);
auto slash=z.parse("<section/>inside</section>");assert(slash->getElementsByTagName("section").front()->textContent()=="inside");
std::cout<<"DOM and HTML parser tests passed.\n";}
