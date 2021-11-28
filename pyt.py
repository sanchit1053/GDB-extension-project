import gdb.printing
import json

normalcolor = 'blue'
selectedcolor = 'green'

# https://stackoverflow.com/questions/57147836/indexing-c-vector-in-python-gdb-script
# input a gdb vector and returns a python list
def vector_to_list(std_vector):
    out_list = []
    value_reference = std_vector['_M_impl']['_M_start']
    while value_reference != std_vector['_M_impl']['_M_finish']:
        out_list.append(value_reference.dereference())
        value_reference += 1

    return out_list

# Graph pretty printer
class graphPrint:
    def __init__(self,val):
        self.val = val;

    def to_string(self):
        nodes = "" # nodes string 
        edges = "" # edges string
        # Read the config file to learn the variable names and other options
        with open('config.json', 'r') as file:
            config = json.load(file) 
        # make the vectors into nodes 
        type = config[0]["type"]
        S = gdb.lookup_type(config[type]["Nodes"])
        if type == 1:
            nodevector = vector_to_list(self.val[config[type]["nodes"]])
            edgevector = vector_to_list(self.val[config[type]["edges"]])
            
            # print("hello ");
            # print(S.keys());
            
            data = []
            for i in nodevector:
                # add the key value of each node in the list to be printed
                nodes += str(i[config[type]["keys"]]) + ', ' 
                # add formatted data into a data list to be sent to a json file
                # [
                    # {
                        # "data" : {
                        #               "id" : <key_value>,
                        #               "color": normalcolor
                        # }
                    # }
                # ] 
                info = ""
                for variable in S:
                    info += '"' + str(variable) + '":"' + str(i[variable]) + '", '
                info = info[0:-2]
                info = "{" + info + "}"
                data.append( {"data":{"id":int(i[config[type]["keys"]]), "color": normalcolor, "info": info}})

            
            for i in edgevector:
                # add each edge iin a string to be printed
                #(key1, key2) , 
                edges += "("  + str(i["first"][config[type]["keys"]])+"," + str(i["second"][config[type]["keys"]])+ '), '
                # adding data to data with format
                # [
                #   {
                        #"data" : {
                        #               "id": <key1><key2>
                        #               "source": <key1>
                        #               "target": <key2>
                        # }
                #   }
                # ] 
                data.append( {"data" : {    "id" : str(i["first"][config[type]["keys"]]) + str(i["second"][config[type]["keys"]]),
                                            "source" : int(i["first"][config[type]["keys"]]),
                                            "target" : int(i["second"][config[type]["keys"]])  }})

            # printing a data to the file to be used to make graphs        
            with open("sample.json","w") as file:
                json.dump(data, file , indent = 4)
            # return nodes and edges to be printned on terminal
            return "nodes: " + nodes + '\n' + "edges: " + edges

        elif type == 2:
            nodevector = vector_to_list(self.val[config[type]["nodes"]])

            data = []
            for node in nodevector:
                nodes += str(node[config[type]["keys"]]) + ','
                info = ""
                for variable in S:
                    info += '"'+ str(variable) + '":"' + str(node[variable]) + '", '
                info = info[0:-2]
                info = "{" + info + "}"
                data.append( {"data":{"id":int(node[config[type]["keys"]]), "color": normalcolor, "info" : info}})
                # print(node[config[type]["neighbour"]])
                adjacent = vector_to_list(node[config[type]["neighbour"]])
                # print("MAMAMAM")
                # print(adjacent)
                for neighbour in adjacent:
                    edges += "(" + str(node[config[type]["keys"]])+"," + str(neighbour[config[type]["keys"]])+ '), '
                    data.append( {"data" : {    "id" : str(node[config[type]["keys"]]) + str(neighbour[config[type]["keys"]]),
                                            "source" : int(node[config[type]["keys"]]),
                                            "target" : int(neighbour[config[type]["keys"]])  }})
                
            with open("sample.json","w") as file:
                json.dump(data, file , indent = 4)
            
            return "nodes: " + nodes + "\n" + "edges: " + edges


    
# Pretty printer for nodes
class NodeMarker:
    def __init__(self, val):
        self.val = val;

    # modify the sample.json file to change color of some nodes
    def to_string(self):
        
        try:
            with open('sample.json', "r") as file:
                data = json.load(file)    
        except:
            return "file not found"
        
        # opening the confing.json files to get the variables name
        with open('config.json', 'r') as file:
            config = json.load(file)

        type = config[0]["type"]

        for node in data:
            # only edges will have source 
            if "source" not in node["data"]:
                # if required node then change the color 
                if node["data"]["id"] == self.val[config[type]["keys"]]:
                    node["data"]["color"] = selectedcolor
                    break
        # output the modified data to the file 
        with open("sample.json","w") as file:
            json.dump(data, file , indent = 4)
        # return the value of the node selected
        return self.val[config[type]["keys"]]

def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("my_library")
    # opening the config.json to get name of variables
    with open('config.json', 'r') as file:
        config = json.load(file)
    # regex for the variables
    type = config[0]["type"]
    if type == 1: 
        node = '^' + config[type]["Nodes"] + '$'
        graph = '^' + config[type]["Graph"] + '$'
    elif type == 2:
        node = '^' + config[type]["Nodes"] + '$'
        graph = '^' + config[type]["Graph"] + '$'
    # adding printers
    pp.add_printer('graph', graph, graphPrint)
    pp.add_printer('node', node, NodeMarker)
    return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(),build_pretty_printer(), replace = True)

