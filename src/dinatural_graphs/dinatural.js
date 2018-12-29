import React from 'react';
import ReactDOM from 'react-dom';
import PropTypes from 'prop-types';
import * as d3 from 'd3';

const data = {
  nodes: [
    { id: 'C', radius: 5, group: 1 },
    { id: 'D', radius: 5, group: 2 },
    { id: 'C2', radius: 0, group: 1 },
    { id: 'E', radius: 5, group: 3 },
    { id: 'F', radius: 5, group: 4 },
  ],
  curved_down_links: [
    { source: 'E', target: 'F', value: 3 },
  ],
  curved_up_links: [
    { source: 'C', target: 'D', value: 3 },
  ],
  straight_links: [

  ],
  invisible_links: [
    { source: 'C', target: 'C2', value: 3 },
    { source: 'D', target: 'C2', value: 3 },
    { source: 'E', target: 'C2', value: 3 },
    { source: 'F', target: 'C2', value: 3 },
  ],
};

function constructArcString(x1, y1, x2, y2, radius) {
  return `M ${x1} ${y1} A ${radius} ${radius} 0 0 0 ${x2} ${y2}`;
}

function constructUpArcFrom(link) {
  return constructArcString(link.source.x, link.source.y,
    link.target.x, link.target.y, 3);
}

function constructDownArcFrom(link) {
  return constructArcString(link.target.x, link.target.y,
    link.source.x, link.source.y, 3);
}

function createDinaturalNodes(nodes, simulation, svg) {
  function dragStarted(d) {
    if (!d3.event.active) simulation.alphaTarget(0.3).restart();
    d.fx = d.x;
    d.fy = d.y;
  }

  function dragged(d) {
    d.fx = d3.event.x;
    d.fy = d3.event.y;
  }

  function dragEnded(d) {
    if (!d3.event.active) simulation.alphaTarget(0);
    d.fx = null;
    d.fy = null;
  }

  return svg.append('g')
    .attr('class', 'nodes')
    .selectAll('circle')
    .data(nodes)
    .enter()
    .append('circle')
    .attr('r', d => d.radius)
    .call(d3.drag()
      .on('start', dragStarted)
      .on('drag', dragged)
      .on('end', dragEnded));
}

function createCurvedUpLinks(links, svg) {
  return svg.append('g')
    .attr('class', 'links')
    .selectAll('path')
    .data(links)
    .enter()
    .append('path');
}

function createCurvedDownLinks(links, svg) {
  return svg.append('g')
    .attr('class', 'links')
    .selectAll('path')
    .data(links)
    .enter()
    .append('path');
}

function createStraightLinks(links, svg) {
  return svg.append('g')
    .attr('class', 'links')
    .selectAll('line')
    .data(links)
    .enter()
    .append('line');
}

function createInvisibleLinks(links, svg) {
  return svg.append('g')
    .attr('class', 'links')
    .selectAll('line')
    .data(links)
    .enter()
    .append('line');
}

function runSimulation(graph, svg, simulation) {
  const straightLinks = createStraightLinks(graph.straight_links, svg);
  const invisibleLinks = createInvisibleLinks(graph.invisible_links, svg);
  const curvedUpLinks = createCurvedUpLinks(graph.curved_up_links, svg);
  const curvedDownLinks = createCurvedDownLinks(graph.curved_down_links, svg);
  const nodes = createDinaturalNodes(graph.nodes, simulation, svg);

  nodes.append('title')
    .text(d => d.id);

  simulation
    .nodes(graph.nodes)
    .on('tick', () => {
      straightLinks
        .attr('x1', d => d.source.x)
        .attr('y1', d => d.source.y)
        .attr('x2', d => d.target.x)
        .attr('y2', d => d.target.y);

      invisibleLinks
        .attr('x1', d => d.source.x)
        .attr('y1', d => d.source.y)
        .attr('x2', d => d.target.x)
        .attr('y2', d => d.target.y);

      curvedUpLinks
        .attr('d', d => constructUpArcFrom(d));

      curvedDownLinks
        .attr('d', d => constructDownArcFrom(d));

      nodes
        .attr('cx', d => d.x)
        .attr('cy', d => d.y);
    });

  simulation.force('link')
    .links(graph.straight_links)
    .links(graph.invisible_links)
    .links(graph.curved_up_links)
    .links(graph.curved_down_links);
}

class DinaturalDiagram extends React.Component {
  drawDiagram() {
    const svg = d3.select('body')
      .append('svg')
      .attr('width', this.props.width)
      .attr('height', this.props.height);

    const simulation = d3.forceSimulation()
      .force('link', d3.forceLink().id(d => d.id))
      .force('charge', d3.forceManyBody())
      .force('center', d3.forceCenter(this.props.width / 2, this.props.height / 2));

    runSimulation(this.props.data, svg, simulation);
  }

  componentDidMount() {
    this.drawDiagram();
  }

  render() {
    return <div id={`#${this.props.id}`}></div>;
  }
}

DinaturalDiagram.propTypes = {
  width: PropTypes.number.isRequired,
  height: PropTypes.number.isRequired,
  data: PropTypes.object.isRequired,
  id: PropTypes.string.isRequired,
};

function test() {
  ReactDOM.render(<DinaturalDiagram id="dd" data={data} width={960} height={600}/>, document.getElementById('root'));
}

export default test;
