import { Component } from 'react';
import { DragSource } from 'react-dnd';
import { Selection } from './selection';
import { PetriTypeComponent } from './petri_type_component';
import { IPetriNet } from '../petri_nets/petri_net_diagram';
import ItemTypes from './item_types';

interface IPetriComponent {
  readonly liveColour: string;
  readonly deadColour: string;
  readonly placeSize: number;
  readonly transitionSize: number;
  readonly selected: Selection;
  readonly cospan: string;
  readonly variable: string;
  readonly transform: string;
  readonly graph: IPetriNet;
  readonly setCospan: (x: string) => any;
  readonly setTransform: (x:string) => any;
  readonly setVariable: (x: string) => any;
  readonly setSVGReference: (x: SVGSVGElement) => void;
};

interface IDraggable {
  readonly connectDragSource?: any;
  readonly connectDragPreview?: any;
  readonly isDragging?: boolean;
};

interface IDraggableTypeComponentProps extends IPetriComponent, IDraggable {
  readonly index: number;
  readonly width: number;
  readonly height: number;
  readonly top: number;
  readonly left: number;
  readonly selected: Selection;
  readonly onClicked: (shift: boolean) => void;
}

const PetriTypeSource = {
  beginDrag(props: IDraggableTypeComponentProps) {
    const { index, left, top } = props;
    return { index, left, top }
  },
};

class DraggablePetriComponent extends Component<IDraggableTypeComponentProps, {}> {

  render() {
    const { isDragging, connectDragPreview, connectDragSource } = this.props;
    if (isDragging) {
      return null;
    }

    return connectDragPreview(
      <div
        className='petricomponent'
        style={{ 
          width:this.props.width, 
          height:this.props.height,
          top:this.props.top,
          left:this.props.left,
        }}
        onClick={(e) => this.props.onClicked(e.shiftKey)}
      >
        {connectDragSource(
          <div 
            className='handle' 
            style={{width: this.props.width}}
          />
        )}
        <PetriTypeComponent
          width={this.props.width}
          height={this.props.height}
          liveColour={this.props.liveColour}
          deadColour={this.props.deadColour}
          placeSize={this.props.placeSize}
          transitionSize={this.props.transitionSize}
          selected={this.props.selected}
          cospan={this.props.cospan}
          variable={this.props.variable}
          graph={this.props.graph}
          transform={this.props.transform}
          setCospan={this.props.setCospan}
          setTransform={this.props.setTransform}
          setVariable={this.props.setVariable}
          setSVGReference={this.props.setSVGReference}
        />
      </div>
    );
  }

}

const collect = (connect, monitor) => ({
  connectDragPreview: connect.dragPreview(),
  connectDragSource: connect.dragSource(),
  isDragging: monitor.isDragging()
});

const draggableNet = DragSource(ItemTypes.PETRI_NET, PetriTypeSource, collect)(DraggablePetriComponent);
export {draggableNet as PetriTypeComponent};