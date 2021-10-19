import gdb.printing
import json

normalcolor = 'blue'
selectedcolor = 'green'

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
        with open('config.json', 'r') as file:
            config = json.load(file)
        nodevector = vector_to_list(self.val[config[0]["nodes"]])
        edgevector = vector_to_list(self.val[config[0]["edges"]])
        data = []
        for i in nodevector:
            nodes += str(i[config[0]["keys"]]) + ', ' 
            data.append( {"data":{"id":int(i[config[0]["keys"]]), "color": normalcolor}})

        
        for i in edgevector:
            edges += "("  + str(i["first"][config[0]["keys"]])+"," + str(i["second"][config[0]["keys"]])+ '), ' 
            data.append( {"data" : {    "id" : str(i["first"][config[0]["keys"]]) + str(i["second"][config[0]["keys"]]),
                                        "source" : int(i["first"][config[0]["keys"]]),
                                        "target" : int(i["second"][config[0]["keys"]])  }})
        
        with open("sample.json","w") as file:
            json.dump(data, file , indent = 4)
        return nodes + "\n" + edges
    

class NodeMarker:
    def __init__(self, val):
        self.val = val;

    def to_string(self):
        try:
            with open('sample.json', "r") as file:
                data = json.load(file)    
        except:
            return "file not found"
        
        with open('config.json', 'r') as file:
            config = json.load(file)
        for node in data:
            if "source" not in node["data"]:
                if node["data"]["id"] == self.val[config[0]["keys"]]:
                    node["data"]["color"] = selectedcolor
                    break
        with open("sample.json","w") as file:
            json.dump(data, file , indent = 4)
        return self.val[config[0]["keys"]]

def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("my_library")
    with open('config.json', 'r') as file:
        config = json.load(file)
    node = '^' + config[0]["Nodes"] + '$'
    graph = '^' + config[0]["Graph"] + '$'
    pp.add_printer('graph', graph, graphPrint)
    pp.add_printer('node', node, NodeMarker)
    return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(),build_pretty_printer(), replace = True)

