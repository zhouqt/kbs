<?php
/*
 Require PHP5, uses built-in DOM extension.
 To be used in PHP4 scripts using DOM_XML extension.
 Allows PHP4/DOMXML scripts to run on PHP5/DOM

 Typical use:
 {
  if (version_compare(PHP_VERSION,'5','>='))
   require_once('domxml-php4-to-php5.php');
 }

 Version 1.1, 2004-07-13, http://alexandre.alapetite.net/doc-alex/domxml-php4-php5/

 ------------------------------------------------------------------
 Written by Alexandre Alapetite, http://alexandre.alapetite.net/cv/

 Copyright 2004, Licence: Creative Commons "Attribution-ShareAlike 2.0" BY-SA,
 http://creativecommons.org/licenses/by-sa/2.0/
 http://alexandre.alapetite.net/divers/apropos/#by-sa
 - Attribution. You must give the original author credit
 - Share Alike. If you alter, transform, or build upon this work,
   you may distribute the resulting work only under a license identical to this one
 - Any of these conditions can be waived if you get permission from Alexandre Alapetite
 - Please send to Alexandre Alapetite the modifications you make,
   in order to improve this file for the benefit of everybody

 If you want to distribute this code, please do it as a link to:
 http://alexandre.alapetite.net/doc-alex/domxml-php4-php5/
*/

function domxml_open_file($filename) {return new php4DOMDocument($filename);}
function xpath_eval($xpath_context,$eval_str) {return $xpath_context->query($eval_str);}
function xpath_new_context($dom_document) {return new php4DOMXPath($dom_document);}

class php4DOMAttr extends php4DOMNode
{
 var $myDOMAttr;
 function php4DOMAttr($aDOMAttr) {$this->myDOMAttr=$aDOMAttr;}
 function Name() {return $this->myDOMAttr->name;}
 function Specified() {return $this->myDOMAttr->specified;}
 function Value() {return $this->myDOMAttr->value;}
}

class php4DOMCDATASection extends php4DOMNode
{
 var $myDOMCDATASection;
 function php4DOMCDATASection($aDOMCDATASection) {$this->myDOMCDATASection=$aDOMCDATASection;}
}

class php4DOMDocument
{
 var $myDOMDocument;
 function php4DOMDocument($filename)
 {
  $this->myDOMDocument=new DOMDocument();
  $this->myDOMDocument->load($filename);
 }
 function create_attribute($name,$value)
 {
  $myAttr=$this->myDOMDocument->createAttribute($name);
  $myAttr->value=$value;
  return new php4DOMAttr($myAttr);
 }
 function create_cdata_section($content) {return new php4DOMCDATASection($this->myDOMDocument->createCDATASection($content));}
 function create_comment($data) {return new php4DOMElement($this->myDOMDocument->createComment($data));}
 function create_element($name) {return new php4DOMElement($this->myDOMDocument->createElement($name));}
 function create_text_node($content) {return new php4DOMNode($this->myDOMDocument->createTextNode($content));}
 function document_element() {return new php4DOMElement($this->myDOMDocument->documentElement);}
 function dump_file($filename,$compressionmode=false,$format=false) {return $this->myDOMDocument->save($filename);}
 function dump_mem($format=false,$encoding=false) {return $this->myDOMDocument->saveXML();}
 function get_elements_by_tagname($name)
 {
  $myDOMNodeList=$this->myDOMDocument->getElementsByTagName($name);
  $nodeSet=array();
  $i=0;
  while ($node=$myDOMNodeList->item($i))
  {
   $nodeSet[]=new php4DOMElement($node);
   $i++;
  }
  return $nodeSet;
 }
 function html_dump_mem() {return $this->myDOMDocument->saveHTML();}
}

class php4DOMElement extends php4DOMNode
{
 function get_attribute($name) {return $this->myDOMNode->getAttribute($name);}
 function get_elements_by_tagname($name)
 {
  $myDOMNodeList=$this->myDOMNode->getElementsByTagName($name);
  $nodeSet=array();
  $i=0;
  while ($node=$myDOMNodeList->item($i))
  {
   $nodeSet[]=new php4DOMElement($node);
   $i++;
  }
  return $nodeSet;
 }
 function has_attribute($name) {return $this->myDOMNode->hasAttribute($name);}
 function remove_attribute($name) {return $this->myDOMNode->removeAttribute($name);}
 function set_attribute($name,$value) {return $this->myDOMNode->setAttribute($name,$value);}
 function tagname() {return $this->myDOMNode->tagName;}
}

class php4DOMNode
{
 var $myDOMNode;
 function php4DOMNode($aDomNode) {$this->myDOMNode=$aDomNode;}
 function append_child($newnode) {return new php4DOMElement($this->myDOMNode->appendChild($newnode->myDOMNode));}
 function append_sibling($newnode) {return new php4DOMElement($this->myDOMNode->parentNode->appendChild($newnode->myDOMNode));}
 function attributes()
 {
  $myDOMNodeList=$this->myDOMNode->attributes;
  $nodeSet=array();
  $i=0;
  while ($node=$myDOMNodeList->item($i))
  {
   $nodeSet[]=new php4DOMAttr($node);
   $i++;
  }
  return $nodeSet;
 }
 function child_nodes()
 {
  $myDOMNodeList=$this->myDOMNode->childNodes;
  $nodeSet=array();
  $i=0;
  while ($node=$myDOMNodeList->item($i))
  {
   $nodeSet[]=new php4DOMElement($node);
   $i++;
  }
  return $nodeSet;
 }
 function clone_node($deep=false) {return new php4DOMElement($this->myDOMNode->cloneNode($deep));}
 function first_child() {return new php4DOMElement($this->myDOMNode->firstChild);}
 function get_content() {return $this->myDOMNode->textContent;}
 function has_attributes() {return $this->myDOMNode->hasAttributes();}
 function has_child_nodes() {return $this->myDOMNode->hasChildNodes();}
 function insert_before($newnode,$refnode) {return new php4DOMElement($this->myDOMNode->insertBefore($newnode->myDOMNode,$refnode->myDOMNode));}
 function last_child() {return new php4DOMElement($this->myDOMNode->lastChild);}
 function next_sibling() {return new php4DOMElement($this->myDOMNode->nextSibling);}
 function node_name() {return $this->myDOMNode->nodeName;}
 function node_type() {return $this->myDOMNode->nodeType;}
 function node_value() {return $this->myDOMNode->nodeValue;}
 function parent_node() {return new php4DOMElement($this->myDOMNode->parentNode);}
 function previous_sibling() {return new php4DOMElement($this->myDOMNode->previousSibling);}
 function remove_child($oldchild) {return new php4DOMElement($this->myDOMNode->removeChild($oldchild->myDOMNode));}
 function replace_child($oldnode,$newnode) {return new php4DOMElement($this->myDOMNode->replaceChild($oldchild->myDOMNode,$newnode->myDOMNode));}
 function set_content($text) {return $this->myDOMNode->textContent=$text;}
}

class php4DOMNodelist
{
 var $myDOMNodelist;
 var $nodeset;
 function php4DOMNodelist($aDOMNodelist)
 {
  $this->myDOMNodelist=$aDOMNodelist;
  $this->nodeset=array();
  $i=0;
  while ($node=$this->myDOMNodelist->item($i))
  {
   $this->nodeset[]=new php4DOMElement($node);
   $i++;
  }
 }
}

class php4DOMXPath
{
 var $myDOMXPath;
 function php4DOMXPath($dom_document) {$this->myDOMXPath=new DOMXPath($dom_document->myDOMDocument);}
 function query($eval_str) {return new php4DOMNodelist($this->myDOMXPath->query($eval_str));}
 function xpath_register_ns($prefix,$namespaceURI) {return $this->myDOMXPath->registerNamespace($prefix,$namespaceURI);}
}

?>
