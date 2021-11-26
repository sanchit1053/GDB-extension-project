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
    }],
    elements:  JSON.parse(data)
      });
        function updateDiv() {$("#cy").load(location.href + " #cy") };
      });
    },{
      greetings: 'Welcome to the GDB Graph visualizer by Team Terminal Stack'
    });

 });