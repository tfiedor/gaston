import gdb
import gdb.printing
import re

__author__ = 'Raph'


class TermProductPrinter(object):
    """
    Pretty printer for 'TermProduct' type of terms.
    """
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return "{" + (term_lookup_function(self.val['left'].dereference())).to_string() + \
               ", " + (term_lookup_function(self.val['right'].dereference()).to_string()) + "}"

    def display_hint(self):
        return "array"


class TermBaseSetPrinter(object):
    """
    Pretty printer for 'TermBaseSet' type of terms.
    """
    def __init__(self, val):
        self.val = val
        self.vector = self.val['states']['vec_']['_M_impl']
        self.vector_len = int(self.vector['_M_finish'] - self.vector['_M_start'])
        self.vector_begin = self.vector['_M_start']

    def to_string(self):
        base_set = "{"
        for i in range(0, self.vector_len):
            base_set += ", "*(i != 0)
            base_set += str(self.vector_begin[i])
        base_set += "}"
        return base_set


class TermFixpointPrinter(object):
    """
    Pretty printer for 'TermFixpoint' type of terms.
    """
    def __init__(self, val):
        self.val = val
        self.fixpoint = self.val['_fixpoint']['_M_impl']
        self.member_type = self.val['_fixpoint'].type.template_argument(0)
        self.head = self.fixpoint['_M_node']

    def to_string(self):
        fixpoint = "{"
        current = self.head['_M_next']
        size = 0
        node_type = gdb.lookup_type('std::_List_node<{}>'.format(self.member_type)).pointer()
        while current != self.head.address:
            node = current.cast(node_type).dereference()['_M_data']
            if node['first'] != 0:
                fixpoint += ", "*(size > 0)
                fixpoint += str(node['first'].dereference())
                fixpoint += " (invalid) "*(str(node['second']) == "false")
                size += 1
            current = current['_M_next']
        fixpoint += "}"
        return fixpoint


class DefaultPrinter(object):
    """
    Default printer for all kinds of shit
    """
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return ""


def is_term(val, term_type):
    return re.match('^Term$', val.type.tag) and str(val['type']) == str(term_type)


def term_lookup_function(val):
    lookup_tag = val.type.tag
    if lookup_tag is None:
        return None
    else:
        if re.match('^TermProduct$', lookup_tag) or is_term(val, 'TermType::PRODUCT'):
            return TermProductPrinter(val.cast(gdb.lookup_type('TermProduct')))
        elif re.match('^TermBaseSet$', lookup_tag) or is_term(val, 'TermType::BASE'):
            return TermBaseSetPrinter(val.cast(gdb.lookup_type('TermBaseSet')))
        elif re.match('^TermFixpoint$', lookup_tag) or is_term(val, 'TermType::FIXPOINT'):
            return TermFixpointPrinter(val.cast(gdb.lookup_type('TermFixpoint')))
        else:
            return None


def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("")
    pp.add_printer('TermProduct', '^TermProduct$', TermProductPrinter)
    pp.add_printer('TermBaseSet', '^TermBaseSet$', TermBaseSetPrinter)
    pp.add_printer('TermFixpoint', '^TermFixpoint$', TermFixpointPrinter)
    return pp


def register_printers(objfile):
    objfile.pretty_printers.append(term_lookup_function)

register_printers(gdb)
