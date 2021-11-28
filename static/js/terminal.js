var f;
var made = false;
function getOption() {
  var e = document.getElementById("mySelect");
  var strUser = e.options[e.selectedIndex].value;
  document.getElementById("info").innerHTML = Object.values(JSON.parse(f))[strUser];
}

$(document).ready(function(){
     
    $('#terminal').terminal(function(command,term){
      $.post('/', {cmd: command}).then(function(r){
        console.log(r.response);
        for (let a in r.response){
            // print all the responses values with console key
            if(r.response[a].type == 'console'){
              // console.log("Hello");
              term.echo(r.response[a].payload);
            }
        }
        var data = JSON.stringify(r.data);
        var cy = cytoscape({
       // container: document.getElementById('cy'),
        container: $('#cy'),

    layout: {
      name: 'dagre',
    },
        style: [
    {
        selector: 'node',
        style: {
            shape: 'round-rectangle',
            'background-color': 'data(color)',
            label: 'data(id)'
          }
    },
    {  selector: 'node:selected',
              css: {
                'line-color': 'black',
                'target-arrow-color': 'black',
                "curve-style": "bezier",
                "control-point-step-size": "100",
                'source-arrow-color': 'black',
                "background-color":'red',
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

    elements:  JSON.parse(data)
      });

        cy.on('click', 'edge', function(evt){
          document.getElementById("info").innerHTML = this.source().id()+"-->"+this.target().id();
        });

      cy.on('tap', 'node', function(evt){
          // document.getElementById("info").innerHTML = this.data("info");

          f = this.data("info")
          console.log(f)
        //   var t = cy.elements('node');
        //   t.style('background-color', 'blue');
        //   this.connectedEdges().targets().style('background-color', 'yellow');    
        //   this.connectedEdges().sources().style('background-color', 'purple');
        var t = cy.elements('node')

        if(!made){

          made = true;
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

        
      //   if (this.scratch().restData == null) {
      //     // Save node data and remove
      //     this.scratch({
      //       restData: this.successors().targets().remove()
      //     });
      //  } else {
      //     // Restore the removed nodes from saved data
      //     this.scratch().restData.restore();
      //     this.scratch({
      //          restData: null
      //     }
      //     );
      //  }         
        // //var t = cy.elements('edge[ source = "1"]');
        //var f = t.targets();
        //f.css('background-color', 'pink');

    });
    cy.on('cxttap', 'node', function(evt){

      var p = this.successors().targets().length;
      if (this.scratch().restData == null) {
        // Save node data and remove
        
        this.scratch({
    
          restData: this.successors().targets().remove()

        });

      if(p > 0){
          this.style('background-color', 'red'); 
          this.style('shape', 'round-pentagon');
          this.style('label', "Group "+String(this.id())+" (Nodes: "+String(p)+")");
      }

      } 
      else {
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

        function updateDiv() {$("#cy").load(location.href + " #cy") };
      });
    },{
      greetings: 'Welcome to the GDB Graph Debugger by Team Terminal Stack'
    });

 });