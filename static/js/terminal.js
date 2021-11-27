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
          document.getElementById("info").innerHTML = 'Data in node: '+this.data("info");
          var t = cy.elements('node');
          t.style('background-color', 'blue');
          this.connectedEdges().targets().style('background-color', 'yellow');    
          this.connectedEdges().sources().style('background-color', 'purple');
          if (this.scratch().restData == null) {
            // Save node data and remove
            this.scratch({
              restData: this.successors().targets().remove()
            });
         } else {
            // Restore the removed nodes from saved data
            this.scratch().restData.restore();
            this.scratch({
                 restData: null
            });
         }
          /*(var nodes = evt.target;
          var tapped = nodes;
          var food = []; 
          nodes.addClass('eater');
          
          for(;;){
            var connectedEdges = nodes.connectedEdges(function(el){
              return !el.target().anySame( nodes );
          });

          var connectedNodes = connectedEdges.targets();

          Array.prototype.push.apply( food, connectedNodes );

          nodes = connectedNodes;
    
    if( nodes.empty() ){ break; }
  }

  var delay = 0;
  var duration = 500;
  for( var i = food.length - 1; i >= 0; i-- ){ (function(){
    var thisFood = food[i];
    var eater = thisFood.connectedEdges(function(el){
      return el.target().same(thisFood);
    }).source();
            
    thisFood.delay( delay, function(){
      eater.addClass('eating');
    } ).animate({
      position: eater.position(),
      css: {
        'width': 10,
        'height': 10,
        'border-width': 0,
        'opacity': 0
      }
    }, {
      duration: duration,
      complete: function(){
        thisFood.remove();
      }
    });
    
    delay += duration;
  })(); } // for
  */
  
 // on tap

        //var t = cy.elements('edge[ source = "1"]');
        //var f = t.targets();
        //f.css('background-color', 'pink');

    });

        function updateDiv() {$("#cy").load(location.href + " #cy") };
      });
    },{
      greetings: 'Welcome to the GDB Graph visualizer by Team Terminal Stack'
    });

 });