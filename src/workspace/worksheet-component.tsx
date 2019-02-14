import reactDraggable from 'react-draggable';

interface IPosition {
  readonly x: number;
  readonly y: number;
}

interface IWorksheetComponentProps {
  readonly element: JSX.Element;
  readonly defaultPosition: IPosition;
  readonly isSelected: boolean;
  readonly onClick: (e: MouseEvent) => any;
}

function getComponentSelector(selected: boolean): string {
  return `worksheet-component${selected ? '-selected' : ''}`;
}

export function WorksheetComponent(props: IWorksheetComponentProps) {
  return (
    <Draggable
      axis="both"
      defaultPosition={props.defaultPosition}
      handle=".handle"
    >
      <div id={getComponentSelector(props.isSelected)} onClick={this.onClick}>
        {props.element}
      </div>
    </Draggable>
  );
}
