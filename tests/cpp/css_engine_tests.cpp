#include "lapislazuli/css/css_engine.hpp"
#include "lapislazuli/dom.hpp"
#include <cassert>
#include <iostream>
#include <utility>
#include <vector>
int main() {
 using namespace lapislazuli;
 dom::Document d;
 auto html=d.createElement("html");
 auto body=d.createElement("body");
 body->setAttribute("class","page dark");
 auto card=d.createElement("div");
 card->setAttribute("id","hero");
 card->setAttribute("class","card featured");
 card->setAttribute("style","border: 1px solid black; color: purple");
 auto title=d.createElement("h1");
 title->setAttribute("class","title");
 card->appendChild(std::move(title));
 body->appendChild(std::move(card));
 auto sized=d.createElement("section");
 sized->setAttribute("class","sized");
 body->appendChild(std::move(sized));
 html->appendChild(std::move(body));
 d.appendChild(std::move(html));
 const auto* cardPtr=d.getElementById("hero");
 auto sheet=css::Parser{}.parse("body { display:block; color:black } .card { color:blue; padding:12px } #hero { color:red } .page > .card.featured { background:#eeeeee; margin:4px } .card .title, h1 { font-size:20px } .card { color:green !important } .sized { font-size:1.5em; padding:.5rem; width:75%; height:120px; margin:1px 2px 3px 4px; box-sizing:border-box }");
 assert(sheet.rules().size()==7);
 auto tokens=css::Tokenizer{}.tokenize("#hero.card > h1.title");
 assert(!tokens.empty()&&tokens.front().type==css::TokenType::Hash&&tokens.front().text=="#hero");
 css::StyleResolver resolver;
 std::vector<const css::StyleSheet*> sheets{&sheet};
 auto tree=resolver.computeTree(d,sheets);
 assert(tree.size()==5);
 const auto& cardStyle=tree.at(cardPtr);
 assert(cardStyle.get("color")&&*cardStyle.get("color")=="green");
 assert(cardStyle.get("padding-top")&&*cardStyle.get("padding-top")=="12px");
 assert(cardStyle.get("background")&&*cardStyle.get("background")=="#eeeeee");
 assert(cardStyle.get("border-top")&&*cardStyle.get("border-top")=="1px");
 const auto* bodyPtr=d.getElementsByTagName("body").front();
 const auto* heading=d.getElementsByTagName("h1").front();
 assert(tree.at(bodyPtr).get("color")&&*tree.at(bodyPtr).get("color")=="black");
 assert(tree.at(heading).get("font-size")&&*tree.at(heading).get("font-size")=="20px");
 assert(tree.at(heading).get("color")&&*tree.at(heading).get("color")=="green");
 const auto* sizedPtr=d.getElementsByTagName("section").front();
 const auto& box=tree.at(sizedPtr).boxModel();
 assert(box.width.value==75&&box.width.unit==css::LengthUnit::Percent);
 assert(tree.at(sizedPtr).length("font-size")->value==1.5&&tree.at(sizedPtr).length("font-size")->unit==css::LengthUnit::Em);
 assert(tree.at(sizedPtr).length("padding-top")->unit==css::LengthUnit::Rem);
 assert(box.height.value==120&&box.height.unit==css::LengthUnit::Px);
 assert(box.margin.left.value==4&&box.margin.right.value==2);
 assert(box.box_sizing=="border-box");
 std::cout<<"CSS values, inheritance, box model, and cascade tests passed.
";
}
