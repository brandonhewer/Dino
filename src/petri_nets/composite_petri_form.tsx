import reactGridLayout, { WidthProvider } from 'react-grid-layout';
import { Selection } from './selection';

const ReactGridLayout = WidthProvider(reactGridLayout);

const formLayout = [
  { i:'transform', x:0, y:0, w:3, h:1 },
  { i:'variable', x:3, y:0, w:1, h:1 },
];

interface ICompositePetriFormProps {
  readonly variable: string;
  readonly transform: string;
  readonly width: number;
  readonly height: number;
  readonly selected: Selection;
  readonly setVariable: (x: string) => void;
}

function getFormClass(selected: Selection): string {
  switch (selected) {
    case Selection.PRIMARY:
      return 'petriform-primary';
    case Selection.SECONDARY:
      return 'petriform-secondary';
    default:
      return 'petriform';
  }
}

export function CompositePetriForm(props: ICompositePetriFormProps) {
  return (
    <form className={getFormClass(props.selected)}>
      <ReactGridLayout
        className="layout"
        layout={formLayout}
        cols={4}
        rowHeight={props.height / 3}
        width={props.width}
        isDraggable={false}
        containerPadding={[0, 0]}
        margin={[0, 0]}
      >
        <div key="transform">
          <textarea readOnly
            className='petri'
            value={props.transform}
          />
        </div>
        <div key="variable">
          <textarea
            className='petri'
            value={props.variable}
            onChange={(e: any) => props.setVariable(e.target.value)}
          />
        </div>
      </ReactGridLayout>
    </form>
  );
}
