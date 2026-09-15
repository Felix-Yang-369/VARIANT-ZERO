"""Run the established independent round-trip and Link checks for V5."""
import os
base=os.path.dirname(os.path.abspath(__file__))
source=open(os.path.join(base,'review_stalker_v4.py'),encoding='utf-8').read().replace('v4','v5').replace('V4','V5')
exec(compile(source,os.path.join(base,'review_stalker_v4.py'),'exec'))
