var f; //stores info or data of node
var made = false; //true when the drop down select option is created

//function to display node data of the selected option in Show
function getOption() {
    var e = document.getElementById("mySelect");
    var strUser = e.options[e.selectedIndex].value;
    document.getElementById("info").innerHTML = Object.values(JSON.parse(f))[strUser];
}

$(document).ready(function() {

    $('#terminal').terminal(function(command, term) {
        $.post('/', { cmd: command }).then(function(r) {
            console.log(r.response);
            for (let a in r.response) {
                // print all the responses values with console key
                if (r.response[a].type == 'console') {
                    term.echo(r.response[a].payload);
                }
            }
            var data = JSON.stringify(r.data);
            var cy = cytoscape({
                container: $('#cy'),

                layout: {
                    name: 'dagre',
                },
                style: [{
                        selector: 'node',
                        style: {
                            shape: 'round-rectangle',
                            'background-color': 'data(color)',
                            label: 'data(id)'
                        }
                    },
                    {
                        selector: 'node:selected',
                        css: {
                            'line-color': 'black',
                            'target-arrow-color': 'black',
                            "curve-style": "bezier",
                            "control-point-step-size": "100",
                            'source-arrow-color': 'black',
                            "border-color": "black",
                            "border-opacity": "1",
                            "border-width": "2px"
                        }
                    },
                    // {  selector: 'edge:selected',
                    //     style: {
                    //       'line-color': 'black',
                    //       'target-arrow-color': 'black',
                    //       "curve-style": "bezier",
                    //       "control-point-step-size": "100",
                    //       width: 4
                    //     }
                    // },
                    // {
                    //   selector: 'edge',
                    //   style: {
                    //     "curve-style": "bezier",
                    //     "control-point-step-size": "100",
                    //     width: 4,
                    //     //"target-arrow-shape": "triangle",
                    //     "line-color": "#9dbaea",
                    //     "target-arrow-color": "#9dbaea"
                    //   }
                    // }
                ],

                elements: JSON.parse(data)
            });

            //Left Click on Edge
            cy.on('click', 'edge', function(evt) {
                //Printing the edge (source->target)
                document.getElementById("info").innerHTML = this.source().id() + "-->" + this.target().id();
            });

            //Left Click on Node
            cy.on('tap', 'node', function(evt) {

                f = this.data("info")
                var t = cy.elements('node');

                //Children Nodes colored yellow
                this.connectedEdges().targets().style('background-color', 'yellow');

                //Parent Nodes colored purple
                this.connectedEdges().sources().style('background-color', 'purple');
                //var t = cy.elements('node')

                if (!made) {

                    made = true;

                    //store the data/info of node(id=1) class inside obj1
                    const obj1 = JSON.parse(cy.elements('node[ id = "1"]').data("info"));

                    var myDiv = document.getElementById("myDiv");
                    //Create array of options to be added
                    var array = Object.keys(obj1);

                    //Create and append select list
                    var selectList = document.createElement("select");
                    selectList.setAttribute("id", "mySelect");
                    myDiv.appendChild(selectList);

                    //Create and append the options
                    for (var i = 0; i < array.length; i++) {
                        var option = document.createElement("option");
                        option.setAttribute("value", i);
                        option.text = array[i];
                        selectList.appendChild(option);
                    }
                }

            });

            //Right Click on Node
            cy.on('cxttap', 'node', function(evt) {

                //All the successors of right-clicked node
                var p = this.successors().targets().length;

                if (this.scratch().restData == null) {
                    // Save node data and remove

                    this.scratch({

                        restData: this.successors().targets().remove()

                    });

                    if (p > 0) {
                        //Style the Group
                        this.style('background-color', 'red');
                        this.style('shape', 'round-pentagon');
                        this.style('label', "Group " + String(this.id()) + " (Nodes: " + String(p) + ")");
                    }

                } else {

                    // Restore the removed nodes from saved data
                    this.scratch().restData.restore();
                    this.style('shape', 'round-rectangle');
                    this.style('label', this.id());
                    this.connectedEdges().sources().style('background-color', 'blue');


                    this.scratch({
                        restData: null
                    });
                }
            });

            function updateDiv() { $("#cy").load(location.href + " #cy") };
        });
    }, {
        greetings: 'Welcome to the GDB Graph Debugger by Team Terminal Stack'
    });

});