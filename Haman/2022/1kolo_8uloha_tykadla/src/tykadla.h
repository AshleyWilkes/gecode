#pragma once

template<typename Topology>
class TykadloData {
  private:
    std::size_t dirIndex;
    int tykadloIndex, maxLength = 0;
    std::vector<std::size_t> fieldIndices;
  public:
    TykadloData() {}
    TykadloData( std::size_t numberIndex, std::size_t dirIndex_, std::size_t fieldIndex,
        const TopologyData<Topology>& givenData ) : dirIndex{ dirIndex_ }, 
        tykadloIndex{ static_cast<int>( ( Topology::DirsCount + 1 ) * numberIndex + dirIndex + 1 ) } {
      //inu zde trochu jinak:
      //zde se deje to, ze vyjdu z nejakeho pole nejakym smerem; na kazdem poli
      //  v danem smeru resim, zda policko je soucast gridu (nevylezl jsem ven)
      //  a zda v policku neni zadane cislo (v givens neni cislo vetsi nez 0)
      //to jsou damn dve operace v sobe, jedna je ten pruchod tim smerem, dokud
      //  jsem uvnitr gridu; druha je, ze kladna cisla jsou blocker, konec
      //  "cesty" v danem smeru;
      //to prvni je operace nad topologii, to druhe uz nikoli, antoz tou prvni
      //  operaci z topologie efektivne vyrabim 1d segment, a ta druha operace
      //  uz predpoklada, ze pracuju s 1d segmentem
      //je otazka, nakolik je treba vsechno doresit zde v hexa tykadlech;
      //  zajimavejsi je otazka, jak pojmout onu prvni operaci s ohledem
      //  na situaci, kterou jednoznacne chci podporovat, namely kdyz
      //  v gridu budou diry (existujici policka, obklopena ze vsech stran
      //  policky gridu, ktera sama, ale nejsou soucasti gridu) a ted si lze
      //  predstavit pravidla, kdy takovato dira uz na urovni topologie
      //  ukoncuje segmenty, anebo pravidla, kdy dira neukoncuje segmenty,
      //  viz kupr. ruzna hypertrofovane geometricka sudoka; tech aspektu se
      //  zde ocividne nabizi slusna radka, tezko je vycerpavajicim zpusobem
      //  pujde popsat, ergo here come ranges/iterators again. Cili to vypada,
      //  ze topologie ma mit metodu, ktera bere startovni policko a smer
      //  a vraci iterator; tento iterator dereferencovan poskytuje info o
      //  indexu pole, kam jsem prisel, jak presne je toto pole soucasti
      //  gridu (gridField, hole, outside). Nad touto metodou zpusobem
      //  analogickym transformacim gridu, jimiz popisuju constrainty, postavim
      //  cokoli budu potrebovat; neni to nakonec analogicke, nybrz presne totez,
      //  jelikoz to, co zde promyslim, je popis constraintu, nikolik gridu.

      //tak to je slohovka a ted co tady:
      //  tady bude topologie vracet pro fieldIndex a dirIndex fieldIndex sousedici
      //  s tim zadanym fieldIndexem v prislusnem smeru; pokud tam uz policko neni
      //  (prislusnym smerem grid konci), vrati se misto fieldIndexu -1
      std::size_t curFieldIndex = fieldIndex, distance = 1;
      while ( ( ( curFieldIndex = givenData.getNeighbor( curFieldIndex, dirIndex ) ) != -1 )
          && ( givenData.getValue( curFieldIndex ) == -1 ) ) {
        fieldIndices.push_back( curFieldIndex );
        maxLength = distance;
        distance++;
      }
      /*typename Topology::Delta dir = Topology::dirs[ dirIndex ];
      int row = fieldIndex / givenData.getWidth(), column = fieldIndex % givenData.getWidth();
      std::size_t curRow = row + dir.row, curColumn = column + dir.column, distance = 1;
      while ( curRow < givenData.getHeight() && curColumn < givenData.getWidth() 
          && givenData.getValue( curRow * givenData.getWidth() + curColumn ) == -1 ) {
        fieldIndices.push_back( curRow * givenData.getWidth() + curColumn );
        maxLength = distance;
        curRow += dir.row;
        curColumn += dir.column;
        distance++;
      }*/
      //print();
    }
    void print() {
      std::cout << "TykadloData(" << tykadloIndex << ", " << dirIndex << ", " << maxLength << "): ";
      for ( auto i : fieldIndices ) {
        std::cout << i << ", ";
      }
      std::cout << '\n';
    }
    int getMaxLength() const { return maxLength; }
    std::vector<std::size_t> getFieldIndices() const { return fieldIndices; }
    Int::IntView getField( const IntVarArray& grid, std::size_t index ) const { 
      //std::cout << fieldIndices[ index ] << ", ";
      return grid[ fieldIndices[ index ] ]; 
    }
    int getTykadloIndex() const { return tykadloIndex; }
    std::size_t getDirIndex() const { return dirIndex; }
};

template<typename Topology>
class GivenNumberData {
  private:
    std::array<TykadloData<Topology>, Topology::DirsCount> tykadlaData;
    int maxLength = 0;
    std::size_t fieldIndex_, numberIndex_, theNumber = 0;
  public:
    GivenNumberData( std::size_t fieldIndex, std::size_t numberIndex,
        const TopologyData<Topology>& givensData )
      : fieldIndex_{ fieldIndex }, numberIndex_{ numberIndex } {
      for ( std::size_t i = 0; i < Topology::DirsCount; ++i ) {
        tykadlaData[ i ] = TykadloData<Topology>{ numberIndex, i, fieldIndex, givensData };
        if ( maxLength < tykadlaData[ i ].getMaxLength() ) {
          maxLength = tykadlaData[ i ].getMaxLength();
        }
      }
      numberIndex_ = numberIndex;
      theNumber = givensData.getValue( fieldIndex );
    }
    int getMaxLength() const { return maxLength; }
    std::array<TykadloData<Topology>, Topology::DirsCount> getTykadlaData() const { return tykadlaData; }
    std::size_t getFieldIndex() const { return fieldIndex_; }
    std::size_t getNumberIndex() const{ return numberIndex_; }
    std::size_t getTheNumber() const { return theNumber; }
};

template<typename Topology>
class Givens {
  private:
    Topology topology;
    TopologyData<Topology> givensData;
    std::vector<GivenNumberData<Topology>> numbersData;
    std::vector<IntArgs> doms;
  public:
    Givens() : topology{}, givensData{} {}
    template<typename GivensInitializerT>
    Givens( const GivensInitializerT& givens ) : topology{ givens }, givensData{ topology, givens } {
      doms.resize( topology.getFieldsCount() );

      for ( std::size_t i = 0; i < topology.getFieldsCount(); ++i ) {
        if ( givensData.getValue( i ) != -1 ) {
          //std::cout << "Given number in " << i << '\n';
          numbersData.push_back({ i, numbersData.size(), givensData });
        }
      }

      for ( auto numberData : numbersData ) {
        doms[ numberData.getFieldIndex() ] << ( Topology::DirsCount + 1 ) * numberData.getNumberIndex();
        //std::cout << "AAA " << ( Topology::DirsCount + 1 ) * numberData.getNumberIndex() << '\n';
        for ( auto tykadloData : numberData.getTykadlaData() ) {
          for ( auto fieldIndex : tykadloData.getFieldIndices() ) {
            doms[ fieldIndex ] << tykadloData.getTykadloIndex();
            //std::cout << "B " << tykadloData.getTykadloIndex() << '\n';
          }
        }
      }
    }
    Topology getTopology() const { return topology; }
    std::size_t getNumbersCount() const { return numbersData.size(); }
    IntArgs getDom( std::size_t index ) const { return doms.at( index ); }
    std::vector<GivenNumberData<Topology>> getNumbersData() const { return numbersData; }
};

template<typename Topology>
class Tykadla : public Script {
private:
  Givens<Topology> givens;
  IntVarArray actualValues;
  //std::size_t givenW, givenH;
public:
  /// Actual model
  Tykadla(const SizeOptions& opt)
      : Script(opt),
      givens{ givensArray }, 
      actualValues(*this, givens.getTopology().getFieldsCount(), 0, 
          ( Topology::DirsCount + 1 ) * givens.getNumbersCount() - 1) {

    for ( std::size_t i = 0; i < givens.getTopology().getFieldsCount(); ++i ) {
      dom( *this, actualValues[ i ], IntSet{ givens.getDom( i ) } );
    }

    for ( const auto& givenNumberData : givens.getNumbersData() ) {
      //std::cout << '\n';
      IntVarArray tykadlaLength{ *this, Topology::DirsCount, 0, givenNumberData.getMaxLength() };
      BoolVarArray tykadlaYesNo{ *this, Topology::DirsCount, 0, 1 };
      for ( int i = 0; i < Topology::DirsCount; ++i ) {
        rel( *this, tykadlaLength[ i ] > 0 == tykadlaYesNo[ i ] );
      }
      IntVar tykadlaCount{ *this, 0, 6 };
      std::cout << "Beware even number of tykadlas required due to 2. kolo, 12 uloha; this breaks 1. kolo 8 uloha (to fix uncomment the following line)\n";
      dom( *this, tykadlaCount, IntSet{ 0, 2, 4, 6 } );
      linear( *this, tykadlaYesNo, IRT_EQ, tykadlaCount );
      for ( const auto& tykadloData : givenNumberData.getTykadlaData() ) {
        if ( tykadloData.getMaxLength() > 0 ) {
          //std::cout << "Tykadlo: ";
          BoolVarArray tykadloBools{ *this, tykadloData.getMaxLength(), 0, 1 };
          for ( int i = 0; i < tykadloData.getMaxLength(); ++i ) {
            rel( *this, ( ( tykadloData.getField( actualValues, i ) == tykadloData.getTykadloIndex() ) 
                  ) == tykadloBools[ i ] );
          }
          //std::cout << '\n';
          TupleSet ts( tykadloData.getMaxLength() );
          for ( std::size_t i = 0; i <= tykadloData.getMaxLength(); ++i ) {
            IntArgs args;
            for ( std::size_t j = 0; j < tykadloData.getMaxLength(); ++j ) {
              args << ( ( j < i ) ? 1 : 0 );
            }
            ts.add( args );
          }
          ts.finalize();
          extensional( *this, tykadloBools, ts );
          linear( *this, tykadloBools, IRT_EQ, tykadlaLength[ tykadloData.getDirIndex() ] );
        } else {
          rel( *this, tykadlaLength[ tykadloData.getDirIndex() ], IRT_EQ, 0 );
        }
      }
      linear( *this, tykadlaLength, IRT_EQ, givenNumberData.getTheNumber() );
      //branch( *this, tykadlaLength, INT_VAR_SIZE_MAX(), INT_VAL_MAX());
    }

    branch(*this, actualValues, INT_VAR_NONE(), INT_VAL_MIN());
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    if constexpr( std::is_same_v<Topology, Rectangle> ) {
      Matrix<IntVarArray> actualValuesM( actualValues, givens.getTopology().getWidth(), 
          givens.getTopology().getHeight() );
      os << actualValuesM << '\n';
    } else {
      givens.getTopology().print( os, actualValues );
    }
  }
  /// Constructor for cloning \a s
  Tykadla(Tykadla& s) : Script(s) {
    actualValues.update(*this, s.actualValues);
    givens = s.givens;
  }
  /// Copy space during cloning
  virtual Space*
  copy(void) {
    return new Tykadla(*this);
  }

};

