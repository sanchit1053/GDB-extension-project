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
      rankDir: 'LR'
    },
        style: [
    {
        selector: 'node',
        style: {
            shape: 'hexagon',
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
                "border-color": "black",
                "border-opacity": "1",
                "border-width": "2px"
              }
          },
          {  selector: 'edge:selected',
              style: {
                'line-color': 'black',
                'target-arrow-color': 'black',
                "curve-style": "bezier",
                "control-point-step-size": "100",
                width: 4
              }
          },
          {
            selector: 'edge',
            style: {
              "curve-style": "bezier",
              "control-point-step-size": "100",
              width: 4,
              //"target-arrow-shape": "triangle",
              "line-color": "#9dbaea",
              "target-arrow-color": "#9dbaea"
            }
          }],

    elements:  JSON.parse(data)
      });


      cy.on('click', 'edge', function(evt){
          document.getElementById("info").innerHTML = this.source().id()+"-->"+this.target().id();
      });

      

      cy.on('tap', 'node', function(evt){
          //document.getElementById("info").innerHTML = 'Data in node: '+this.data("info");
          f = this.data("info");
          var t = cy.elements('node');
          t.style('background-color', 'blue');
          this.connectedEdges().targets().style('background-color', 'yellow');    
          this.connectedEdges().sources().style('background-color', 'purple');  



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

            






      });

      function updateDiv() {$("#cy").load(location.href + " #cy") };

    });
    },
    {
      greetings: 'Welcome to the GDB Graph visualizer by Team Terminal Stack'
    });

 });