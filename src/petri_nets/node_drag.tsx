import * as d3 from 'd3';

export function node_drag(simulation: any): any {
  function onDragStart(d: any) {
    if (!d3.event.active) {
      simulation.alphaTarget(0.3).restart();
    }
    d.fx = d.x;
    d.fy = d.y;
  }

  function onDrag(d: any) {
    d.fx = d3.event.x;
    d.fy = d3.event.y;
  }

  function onDragEnd(d: any) {
    if (!d3.event.active) {
      simulation.alphaTarget(0);
    }
    d.fx = null;
    d.fy = null;
  }

  return d3.drag()
    .on('start', onDragStart)
    .on('drag', onDrag)
    .on('end', onDragEnd);
}
