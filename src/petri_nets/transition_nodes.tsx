import * as d3 from 'd3';
import { Component } from 'react';
import { node_drag } from './node_drag';

interface INode {
  readonly id: number;
}

interface ITransitionProps {
  readonly transitions: INode[];
  readonly width: number;
  readonly height: number;
  readonly simulation: any;
  readonly fillColour: (node: INode) => string;
  readonly onClick: (node: INode, index: number) => any;
}

export class TransitionNodes extends Component<ITransitionProps, {}> {
  private ref: SVGGElement;

  public componentDidMount(): void {
    this.drawNodes();
  }

  public componentDidUpdate(): void {
    this.drawNodes();
  }

  public render(): JSX.Element {
    return <g className="transitions" ref={(ref: SVGGElement) => this.ref = ref}/>;
  }

  private drawNodes(): void {
    const nodes = d3.select(this.ref)
      .selectAll('rect.node')
      .data(this.props.transitions);

    nodes.call(node_drag(this.props.simulation))
      .on('click', this.props.onClick);

    nodes.attr('class', 'node')
      .attr('width', this.props.width)
      .attr('height', this.props.height)
      .style('fill', this.props.fillColour)
      .call(node_drag(this.props.simulation))
      .on('click', this.props.onClick);

    nodes.enter().append('rect')
      .attr('class', 'node')
      .attr('width', this.props.width)
      .attr('height', this.props.height)
      .style('stroke', '#000')
      .style('stroke-width', '2')
      .style('fill', this.props.fillColour)
      .call(node_drag(this.props.simulation))
      .on('click', this.props.onClick);

    nodes.exit().remove();
  }
}
