# Assumes motionSetPoints is running using larmor.cmd 
# and talking to a motor simulator (assumes $(MYPVPREFIX)MTR01)
# This test checks that one IOC can support multiple lookups without crosstalk

import unittest
import os
import time
from shutil import copyfile
from epics import PV

class testLarmorMSP(unittest.TestCase):
	'''Test stuff'''
	DELAY = 2
	
	def test_positions(self):
		PVPREFIX = os.environ['MYPVPREFIX'] + 'LKUP:'
		filter = PV(PVPREFIX + 'SAMPX:FILTER1')
		filter.put('Rack1_Rct')
		posns = PV(PVPREFIX + 'SAMPX:POSITIONS')
		positions = posns.get()
		self.assertEqual(positions[0], 'Rack1_Rct_1')
		self.assertEqual(positions[2], 'Rack1_Rct_3')
		self.assertEqual(positions[10], 'END')
		posns = PV(PVPREFIX + 'MON3:POSITIONS')
		positions = posns.get()
		self.assertEqual(positions[0], 'In')
		self.assertEqual(positions[1], 'Out')
		self.assertEqual(positions[2], 'END')
		posns = PV(PVPREFIX + 'SAMPY:POSITIONS')
		positions = posns.get()
		self.assertEqual(positions[0], 'Top')
		self.assertEqual(positions[1], 'Bottom')
		self.assertEqual(positions[2], 'END')
	
	def test_lookps(self):
		PVPREFIX = os.environ['MYPVPREFIX'] + 'LKUP:'
		posn3 = PV(PVPREFIX + 'MON3:POSN')
		posn4 = PV(PVPREFIX + 'MON4:POSN')
		posnX = PV(PVPREFIX + 'SAMPX:POSN')
		posnY = PV(PVPREFIX + 'SAMPY:POSN')
		posn3sp = PV(PVPREFIX + 'MON3:POSN:SP')
		posn4sp = PV(PVPREFIX + 'MON4:POSN:SP')
		posnXsp = PV(PVPREFIX + 'SAMPX:POSN:SP')
		posnYsp = PV(PVPREFIX + 'SAMPY:POSN:SP')
		print(PVPREFIX + 'MON3:POSN')
		posn3sp.put('Out')
		posn4sp.put('In')
		posnXsp.put('Rack1_Rct_3')
		posnYsp.put('Bottom')
		time.sleep(self.DELAY)
		rbv3 = PV(PVPREFIX + 'MON3:COORD1:RBV')
		rbv4 = PV(PVPREFIX + 'MON4:COORD1:RBV')
		rbvX = PV(PVPREFIX + 'SAMPX:COORD1:RBV')
		rbvY = PV(PVPREFIX + 'SAMPY:COORD1:RBV')
		
		pvStationary = PV(PVPREFIX + 'SAMPX:STATIONARY')
		for i in range(20):
			stationary = pvStationary.get()
			if stationary==1:
				break
			print('Moving')
			print('3=' + str(rbv3.get()) + ' 4=' + str(rbv4.get()) + ' X=' + str(rbvX.get()) + ' Y=' + str(rbvY.get()))
			time.sleep(self.DELAY)

		time.sleep(self.DELAY)
		self.assertEqual(posn3.get(), 'Out')
		self.assertEqual(posn4.get(), 'In')
		self.assertEqual(posnX.get(), 'Rack1_Rct_3')
		self.assertEqual(posnY.get(), 'Bottom')
		coord3 = PV(PVPREFIX + 'MON3:COORD1')
		coord4 = PV(PVPREFIX + 'MON4:COORD1')
		coordX = PV(PVPREFIX + 'SAMPX:COORD1')
		coordY = PV(PVPREFIX + 'SAMPY:COORD1')
		self.assertAlmostEqual(coord3.get(), 10)
		self.assertAlmostEqual(coord4.get(), 5)
		self.assertAlmostEqual(coordX.get(), 30)
		self.assertAlmostEqual(coordY.get(), 12)
		self.assertAlmostEqual(rbv3.get(), 10)
		self.assertAlmostEqual(rbv4.get(), 5)
		self.assertAlmostEqual(rbvX.get(), 30)
		self.assertAlmostEqual(rbvY.get(), 12)
		
