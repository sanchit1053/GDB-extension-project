import gdb.printing
import json
# from 
def vector_to_list(std_vector):
    out_list = []
    value_reference = std_vector['_M_impl']['_M_start']
    while value_reference != std_vector['_M_impl']['_M_finish']:
        out_list.append(value_reference.dereference())
        value_reference += 1

    return out_list

class graphPrint:
    def __init__(self,val):
        self.val = val;

    def to_string(self):
        nodes = ""
        edges = ""
        nodevector = vector_to_list(self.val["nodes"])
        edgevector = vector_to_list(self.val["edges"])
        data = []
        for i in nodevector:
            nodes += str(i["number"]) + ', ' 
            data.append( {"data":{"id":int(i["number"])}})

        
        for i in edgevector:
            edges += "("  + str(i["first"]["number"])+"," + str(i["second"]["number"])+ '), ' 
            data.append( {"data" : {    "id" : str(i["first"]["number"]) + str(i["second"]["number"]),
                                        "source" : int(i["first"]["number"]),
                                        "target" : int(i["second"]["number"])  }})
        
        with open("sample.json","w") as file:
            json.dump(data, file)
        return nodes + "\n" + edges
        #return (str(self.val["nodes"]) + "\n" +  str(self.val["edges"]))
    
def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("my_library")
    pp.add_printer('graph', '^Graph$', graphPrint)
    return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(),build_pretty_printer(), replace = True)